#include <stdlib.h>
#include <valgrind/valgrind.h>
#include <string.h>
#include <signal.h>

#include "logger.h"
#include "timer.h"
#include "queue.h"
#include "dispatcher.h"
#include "ppos.h"
#include "ppos_data.h"

#define STACKSIZE 64*1024

#define MIN_PRIORITY -20
#define MAX_PRIORITY 20

#define QUANTUM_INTERVAL_MS (long)1
#define TASK_QUANTUM (short)20

#define MAX_SKIP_TASK_SWITCH 10

static ppos_core_t* _ppos_core = NULL;
static unsigned int _skip_task_switch_count = 0;
static bool _task_switch_enabled = true;

static void _enable_task_switch()
{
    LOG_TRACE("enable_task_switch: task %d enabling task switch", _ppos_core->current_task->id);
    _task_switch_enabled = true;
}

static void _block_task_switch()
{
    LOG_TRACE("block_task_switch: task %d blocking task switch", _ppos_core->current_task->id);
    _task_switch_enabled = false;
}

static bool _is_task_switch_enabled()
{
    return _task_switch_enabled;
}

static task_t* _setup_task_stack(task_t *task, int stack_size)
{
    stack_t *stack = calloc(1, stack_size);

    if (stack == NULL) {
        LOG_WARN0("setup_task_stack: failed to allocate stack");
        return NULL;
    }

    task->vg_id = VALGRIND_STACK_REGISTER(stack, stack + stack_size);
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = stack_size;
    task->context.uc_stack.ss_flags = 0;

    return task;
}

static void _free_task_stack(task_t *task)
{
    if (task->context.uc_stack.ss_sp)
    {
        //free(task->context.uc_stack.ss_sp);
        VALGRIND_STACK_DEREGISTER(task->vg_id);
    }
}

static void _ppos_destroy()
{
    if (_ppos_core != NULL)
    {
        if (_ppos_core->dispatcher_task != NULL)
        {
            if (_ppos_core->dispatcher_task->context.uc_stack.ss_sp)
            {
                //free(_ppos_core->dispatcher_task->context.uc_stack.ss_sp);
                VALGRIND_STACK_DEREGISTER(_ppos_core->dispatcher_task->vg_id);
            }

            free(_ppos_core->dispatcher_task);
        }

        free(_ppos_core);
    }
}

static task_t* _create_task(task_t* task, task_type_t type, int stack_size, struct ucontext_t *link, void (*start_func)(void *), void *arg)
{
    if (task == NULL) {
        task = calloc(1, sizeof(task_t));

        if (task == NULL) {
            LOG_WARN0("create_task: failed to allocate task");
            return NULL;
        }
    }
    
    task->id = _ppos_core->task_cnt++;
    task->type = type;
    task->quantum = TASK_QUANTUM;
    task->remaining_quantum = TASK_QUANTUM;
    task->time.creation_time = systime();
    
    if (link != NULL) {
        task->context.uc_link = link;
    } else {
        task->context.uc_link = NULL;
    }

    if (getcontext(&task->context) < 0) {
        LOG_WARN0("create_task: failed to get context");
        return NULL;
    }
   
    if (stack_size > 0)
    {
        if (_setup_task_stack(task, stack_size) == NULL)
        {
            LOG_WARN0("create_task: failed to setup stack");
            return NULL;
        }
    }
    
    if (start_func != NULL) {
        makecontext(&task->context, (void (*)(void))start_func, 1, arg);
    }

    task->status = TASK_STATUS_CREATED;
    LOG_INFO("create_task: task %d created", task->id);
    return task;
}

static void _update_total_time(task_t *task) {
    if (task == NULL) {
        LOG_WARN0("update_total_time: task is NULL, skipping");
        return;
    }
    
    if (task->time.last_start == 0) {
        LOG_TRACE("update_total_time: task %d not started since last update, skipping", task->id);
        return;
    }
    
    unsigned int now = systime();
    unsigned int elapsed = now - task->time.last_start;
    task->time.last_start = 0;

    if (elapsed > now) {
        LOG_WARN("update_total_time: overflow on total_cpu_time, setting to max unsigned int");
        elapsed = (unsigned int)-1;
    }

    task->time.total_cpu_time += elapsed;
}

static void _start_timing(task_t *task) {
    if (task == NULL) {
        LOG_WARN0("start_timing: task is NULL, skipping");
        return;
    }
    
    task->time.last_start = systime();
    task->time.activations++;
}

static void _finish_task_timing(task_t *task) {
    _update_total_time(task);
    unsigned int total_time = systime() - task->time.creation_time;
    LOG("Task %d exit: execution time %u ms, processor time %u ms, %u activations", 
         task->id, total_time, task->time.total_cpu_time, task->time.activations);
}

static void _set_current_task(task_t *prev_task, task_t *task)
{
    if (prev_task != NULL) {
        LOG_DEBUG("set_current_task: changing current task from %d to %d", prev_task->id, task->id);
        _update_total_time(prev_task);
    } else {
        LOG_DEBUG("set_current_task: changing current task from NULL to %d", task->id);
    }
    
    if (task == NULL) {
        LOG_WARN0("set_current_task: cannot set current task to NULL, skipping");
        return;
    }
    
    _ppos_core->current_task = task;
    _ppos_core->current_task->status = TASK_STATUS_RUNNING;    
    _start_timing(_ppos_core->current_task);
}

static void _tick_handler(int signum)
{   
    if (signum != timer_signal())
    {
        LOG_WARN("tick_handler: received signal %d, skipping", signum);
        return;
    }
    
    task_t *current_task = _ppos_core->current_task;
    if (current_task == NULL)
    {
        LOG_WARN0("tick_handler: current task is NULL");
        return;
    }
    
    if (!_is_task_switch_enabled())
    {
        LOG_INFO("tick_handler: task switch is disabled, skipping");
        return;
    }

    if (current_task->type == TASK_TYPE_SYSTEM && current_task->status == TASK_STATUS_RUNNING)
    {
        LOG_INFO("tick_handler: skipping preemption for system task %d still running", current_task->id);
        return;
    }

    LOG_TRACE("tick_handler: task %d quantum is %d", current_task->id, current_task->remaining_quantum);
    current_task->remaining_quantum--;

    if (current_task->remaining_quantum <= 0)
    {
        LOG_INFO("tick_handler: task %d quantum expired, yielding", current_task->id);
        current_task->remaining_quantum = current_task->quantum;
        task_yield();
    }
}

static void _awake_all(queue_t **queue)
{
    if (queue == NULL) {
        LOG_WARN("awake_all: cannot awake NULL queue");
        return;
    }
    
    for (queue_t *task = *queue; queue_size(*queue) > 0; task = task->next) {
        task_awake((task_t*)task, (task_t**)(queue));
    }
}

static int _add_task_to_queue(task_t *task, queue_t **queue)
{
    if (queue == NULL) {
        LOG_WARN("add_task_to_queue: cannot add task to NULL queue");
        return -1;
    }
    
    _block_task_switch();
    int ret = queue_append((queue_t **)queue, (queue_t*)task);
    _enable_task_switch();
    return ret;
}

static int _remove_task_from_queue(task_t *task, queue_t **queue)
{
    if (queue == NULL) {
        LOG_WARN("remove_task_from_queue: cannot remove task from NULL queue");
        return -1;
    }
    
    _block_task_switch();
    int ret = queue_remove(queue, (queue_t*)task);
    _enable_task_switch();
    return ret;
}

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);

    _ppos_core = calloc(1, sizeof(ppos_core_t));
    if (_ppos_core == NULL) {
        LOG_ERR0("ppos_init: failed to allocate ppos_core");
        exit(-1);
    }

    _ppos_core->ready_queue = NULL;

    _ppos_core->dispatcher_task = _create_task(
        NULL, 
        TASK_TYPE_SYSTEM, 
        STACKSIZE, 
        NULL, 
        (void (*)(void *))dispatcher,
        &_ppos_core->ready_queue
    );

    if (_ppos_core->dispatcher_task == NULL)
    {
        LOG_ERR0("ppos_init: failed to create dispatcher task");
        exit(-1);
    }

    _ppos_core->main_task = _create_task(
        NULL, 
        TASK_TYPE_USER, 
        0, 
        &_ppos_core->dispatcher_task->context,
        NULL,
        NULL
    );

    if (_ppos_core->main_task == NULL)
    {
        LOG_ERR0("ppos_init: failed to create main task");
        exit(-1);
    }

    timer_init(_tick_handler, QUANTUM_INTERVAL_MS);
    _set_current_task(NULL, _ppos_core->main_task);
    task_yield();
}

int task_init(task_t *task, void (*start_func)(void *), void *arg)
{
    if (task == NULL) {
        LOG_ERR0("task_init: cannot create task with NULL pointer");
        return -1;
    }
    
    task = _create_task(
        task, 
        TASK_TYPE_USER, 
        STACKSIZE, 
        &_ppos_core->dispatcher_task->context,
        start_func,
        arg
    );
    
    if (task == NULL) {
        LOG_ERR0("task_init: failed to create task");
        return -1;
    }

    if (_add_task_to_queue(task, &_ppos_core->ready_queue) < 0) {
        LOG_ERR0("task_init: failed to append task to ready queue");
        return -1;
    }

    LOG_INFO("task_init: task %d initialized", task->id);
    return task->id;
}

int task_id()
{  
    return _ppos_core->current_task->id;
}

void task_exit(int exit_code)
{
    _finish_task_timing(_ppos_core->current_task);

    _ppos_core->current_task->status = TASK_STATUS_TERMINATED;
    _ppos_core->current_task->exit_code = exit_code;
    _free_task_stack(_ppos_core->current_task);
    LOG_DEBUG("task_exit: task %d exited", _ppos_core->current_task->id);

    _awake_all(&_ppos_core->current_task->waiting_queue);

    if (_ppos_core->current_task == _ppos_core->dispatcher_task)
    {
        LOG_DEBUG("task_exit: dispatcher task exiting");
        _ppos_destroy();
        exit(exit_code);
    }

    if (_ppos_core->current_task == _ppos_core->main_task)
    {
        LOG_DEBUG("task_exit: switching from main task to dispatcher on exit");
        task_switch(_ppos_core->dispatcher_task);
    }
}

int task_switch(task_t *task)
{
    if (task == NULL || _ppos_core == NULL || _ppos_core->current_task == NULL) {
        LOG_ERR0("task_switch: invalid task, ppos_core or current_task is NULL, skipping");
        return -1;
    }

    if (_ppos_core->current_task == task) {
        LOG_INFO("task_switch: ignoring switch to same task %d", _ppos_core->current_task->id);
        return 0;
    }

    if (!_is_task_switch_enabled() && _ppos_core->current_task->type != TASK_TYPE_SYSTEM) {
        if (_skip_task_switch_count < MAX_SKIP_TASK_SWITCH) {
            _skip_task_switch_count++;
            LOG_WARN("task_switch: task switch is disabled, cannot switch to task %d", task->id);
            return -1;
        }
        
        LOG_WARN("task_switch: max skip task switch count reached, forcing task switch");
        _enable_task_switch();
    }
    
    _skip_task_switch_count = 0;

    task_t *prev_task = _ppos_core->current_task;

    if (prev_task->id == _ppos_core->dispatcher_task->id) {
        prev_task->status = TASK_STATUS_SUSPENDED;
    }

    _set_current_task(prev_task, task);
    LOG_INFO("task_switch: switching from task %d to task %d", prev_task->id, task->id);

    if (swapcontext(&prev_task->context, &task->context) < 0) {
        LOG_ERR0("task_switch: failed to switch context");
        return -1;
    }
    
    if (task->status == TASK_STATUS_RUNNING)
    {    
        LOG_WARN("task_switch: task %d is still running, should not reach here", task->id);
    }

    _set_current_task(task, prev_task);
    LOG_DEBUG("task_switch: switched back from task %d to task %d", task->id, prev_task->id);

    if (prev_task->id == _ppos_core->dispatcher_task->id) {
        _ppos_core->current_task->status = TASK_STATUS_RUNNING;
    }

    return 0;
}

void task_yield()
{
    LOG_TRACE("task_yield: yielding task %d", _ppos_core->current_task->id);
    _ppos_core->current_task->status = TASK_STATUS_READY;
    _add_task_to_queue(_ppos_core->current_task, &_ppos_core->ready_queue);
    task_switch(_ppos_core->dispatcher_task);
}

void task_setprio(task_t *task, int prio)
{
    if (prio < MIN_PRIORITY)
    {
        LOG_WARN("task_setprio: priority %d is lower than minimum priority %d, setting to minimum priority", prio, MIN_PRIORITY);
        prio = MIN_PRIORITY;
    } 
    else if (prio > MAX_PRIORITY)
    {
        LOG_WARN("task_setprio: priority %d is higher than maximum priority %d, setting to maximum priority", prio, MAX_PRIORITY);
        prio = MAX_PRIORITY;
    }

    if (task == NULL)
    {
        task = _ppos_core->current_task;
    }
    
    LOG_TRACE("task_setprio: setting task %d priority to %d", task->id, prio);

    task->priority = prio;
    task->dynamic_priority = prio;
}

int task_getprio(task_t *task)
{
    if (task == NULL)
    {
        task = _ppos_core->current_task;
    }
    
    LOG_TRACE("task_getprio: getting task %d priority (%d)", task->id, task->priority);
    return task->priority;
}

void task_suspend(task_t **queue)
{
    LOG_INFO("task_suspend: suspending task %d", _ppos_core->current_task->id);
    _remove_task_from_queue(_ppos_core->current_task, &_ppos_core->ready_queue);
    
    if (queue != NULL) {
        _add_task_to_queue(_ppos_core->current_task, (queue_t**)queue);
    }

    _ppos_core->current_task->status = TASK_STATUS_SUSPENDED;
    task_switch(_ppos_core->dispatcher_task);
}

void task_awake(task_t *task, task_t **queue)
{
    if (task == NULL) {
        LOG_ERR("task_awake: cannot awake NULL task");
        return;
    }
    
    LOG_INFO("task_awake: awaking task %d", task->id);
    if (queue != NULL) {
        _remove_task_from_queue(task, (queue_t**)queue);
    }
    
    task->status = TASK_STATUS_READY;
    _add_task_to_queue(task, &_ppos_core->ready_queue);
}

int task_wait(task_t *task)
{
    if (task == NULL) {
        LOG_ERR("task_wait: cannot wait for NULL task");
        return -1;
    }

    LOG_INFO("task_wait: task %d will wait for task %d", _ppos_core->current_task->id, task->id);
    if (task->status != TASK_STATUS_TERMINATED) {
        task_suspend((task_t**)&task->waiting_queue);    
    }

    return task->exit_code;
}
