#include <stdlib.h>
#include <valgrind/valgrind.h>
#include <string.h>

#include "ppos.h"
#include "ppos_data.h"
#include "logger.h"
#include "queue.h"
#include "dispatcher.h"

#define STACKSIZE 64*1024

#define MIN_PRIORITY -20
#define MAX_PRIORITY 20

ppos_core_t* ppos_core = NULL;

static void _setup_task_stack(task_t *task, stack_t *stack)
{
    task->vg_id = VALGRIND_STACK_REGISTER(stack, stack + STACKSIZE);
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
}

static void _ppos_destroy()
{
    if (ppos_core != NULL)
    {
        if (ppos_core->dispatcher_task != NULL)
        {
            if (ppos_core->dispatcher_task->context.uc_stack.ss_sp)
            {
                //free(ppos_core->dispatcher_task->context.uc_stack.ss_sp);
                VALGRIND_STACK_DEREGISTER(ppos_core->dispatcher_task->vg_id);
            }

            free(ppos_core->dispatcher_task);
        }

        free(ppos_core);
    }
}

void ppos_init()
{
    setvbuf(stdout, 0, _IONBF, 0);

    ppos_core = calloc(1, sizeof(ppos_core_t));
    if (ppos_core == NULL) exit(-1);

    ppos_core->dispatcher_task = calloc(1, sizeof(task_t));
    if (ppos_core->dispatcher_task == NULL) exit(-1);

    memset(ppos_core->dispatcher_task, 0, sizeof(task_t));
    memset(&ppos_core->dispatcher_task->context, 0, sizeof(ucontext_t));

    stack_t *dispatcher_stack = calloc(1, STACKSIZE);
    if (!dispatcher_stack) exit(-1);

    ppos_core->dispatcher_task->type = TASK_TYPE_DISPATCHER;
    ppos_core->dispatcher_task->status = TASK_STATUS_READY;

    ppos_core->task_cnt = 1;
    ppos_core->dispatcher_task->id = ppos_core->task_cnt++;
    LOG_INFO("ppos_init: task 1 created");

    _setup_task_stack(ppos_core->dispatcher_task, dispatcher_stack);
    ppos_core->dispatcher_task->context.uc_link = 0;

    if (getcontext(&ppos_core->dispatcher_task->context) < 0) exit(-1);
    makecontext(&ppos_core->dispatcher_task->context, (void (*)(void))dispatcher, 1, &ppos_core->ready_queue);

    ppos_core->current_task = NULL;
    ppos_core->ready_queue = NULL;
}

int task_init(task_t *task, void (*start_func)(void *), void *arg)
{
    if (task == NULL) return -1;
    
    memset(task, 0, sizeof(task_t));
    memset(&task->context, 0, sizeof(ucontext_t));

    task->id = ppos_core->task_cnt++;
    task->type = TASK_TYPE_USER;

    task->priority = 0;
    task->dynamic_priority = 0;

    stack_t *stack = calloc(1, STACKSIZE);
    if (stack == NULL) return -1;

    if (getcontext(&task->context) < 0) return -1;
    _setup_task_stack(task, stack);

    task->context.uc_link = &ppos_core->dispatcher_task->context;
    makecontext(&task->context, (void (*)(void))start_func, 1, arg);
    
    task->status = TASK_STATUS_READY;
    if (queue_append(&ppos_core->ready_queue, (queue_t*)task) < 0) return -1;

    LOG_INFO("task_init: task %d created", task->id);
    return task->id;
}

int task_id ()
{
    return ppos_core->current_task->id;
}

void task_exit(int exit_code)
{

    if (ppos_core->dispatcher_task->status == TASK_STATUS_READY)
    {
        LOG_INFO("task_exit: starting dispatcher context");

        ppos_core->dispatcher_task->status = TASK_STATUS_RUNNING;
        ppos_core->current_task = ppos_core->dispatcher_task;
        setcontext(&ppos_core->dispatcher_task->context);

        LOG_ERR0("task_exit: should not reach here");
        exit(-1);
    }

    if (ppos_core->dispatcher_task->status == TASK_STATUS_RUNNING)
    {
        LOG_INFO("task_exit: finished dispatcher");
        ppos_core->dispatcher_task->status = TASK_STATUS_TERMINATED;
        _ppos_destroy();
        exit(exit_code);
    }

    LOG_INFO("task_exit: task %d finished", ppos_core->current_task->id);
    ppos_core->current_task->status = TASK_STATUS_TERMINATED;
    return;
}

int task_switch(task_t *task)
{
    if (task == NULL || ppos_core == NULL || ppos_core->current_task == NULL) return -1;

    LOG_INFO("task_switch: switching from task %d to task %d", ppos_core->current_task->id, task->id);

    task_t* prev_task = ppos_core->current_task;

    prev_task->status = TASK_STATUS_SUSPENDED;
    task->status = TASK_STATUS_RUNNING;

    ppos_core->current_task = task;

    if (swapcontext(&prev_task->context, &task->context) < 0) {
        LOG_ERR0("task_switch: failed to switch context");
        return -1;
    }
    
    LOG_DEBUG("task_switch: switched back to task %d", prev_task->id);

    if (task->status == TASK_STATUS_RUNNING)
    {    
        LOG_WARN("task_switch: task %d is running", task->id);
    }

    prev_task->status = TASK_STATUS_RUNNING;
    ppos_core->current_task = prev_task;

    return 0;
}

void task_yield ()
{
    LOG_INFO("task_yield: yielding task %d", ppos_core->current_task->id);
    ppos_core->current_task->status = TASK_STATUS_READY;

    queue_append(&ppos_core->ready_queue, (queue_t*)ppos_core->current_task);
    task_switch(ppos_core->dispatcher_task);
}

void task_setprio (task_t *task, int prio)
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
        task = ppos_core->current_task;
    }
    
    LOG_TRACE("task_setprio: setting task %d priority to %d", task->id, prio);

    task->priority = prio;
    task->dynamic_priority = prio;
}

int task_getprio (task_t *task)
{
    if (task == NULL)
    {
        task = ppos_core->current_task;
    }
    
    LOG_TRACE("task_getprio: getting task %d priority (%d)", task->id, task->priority);
    return task->priority;
}