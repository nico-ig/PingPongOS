#include <stdlib.h>
#include <valgrind/valgrind.h>

#include "ppos_core_internal.h"
#include "ppos_data.h"
#include "logger.h"
#include "queue.h"
#include "task.h"
#include "dispatcher.h"

#define STACKSIZE 64*1024

extern ppos_core_t* ppos_core;

int _init_ppos_core()
{
    LOG_TRACE0("_init_ppos_core: entered");

    ppos_core = calloc(1, sizeof(ppos_core_t));
    if (ppos_core == NULL) {
        LOG_WARN("_init_ppos_core: failed to allocate ppos_core");
        return PPOS_CORE_ERR_NULL_PTR;
    }

    LOG_TRACE("_init_ppos_core: exiting (%d)", PPOS_CORE_SUCCESS);
    return 0;
}

int _init_system_task(task_t **task_ptr, task_type_t type, 
                     void (*start_func)(void *), void *arg)
{
    LOG_TRACE0("_init_system_task: entered");

    *task_ptr = calloc(1, sizeof(task_t));
    if (*task_ptr == NULL) {
        LOG_WARN("_init_system_task: failed to allocate task");
        return PPOS_CORE_ERR_NULL_PTR;
    }

    int ret = task_create_context(*task_ptr, start_func, arg);
    if (ret < PPOS_CORE_SUCCESS) {
        LOG_WARN("_init_system_task: failed to create task context");
        free(*task_ptr);
        *task_ptr = NULL;
        return ret;
    }

    (*task_ptr)->type = type;
    _set_task_id(*task_ptr);

    LOG_TRACE("_init_system_task: exiting (%d)", ret);
    return ret;
}

int _init_main_task()
{
    LOG_TRACE0("_init_main_task: entered");
    int ret = _init_system_task(&ppos_core->main_task, TASK_TYPE_MAIN, 
                               _main_task_wrapper, (void *)0);
    
    if (ret < PPOS_CORE_SUCCESS) {
        LOG_WARN("_init_main_task: failed to initialize main task");
        goto exit;
    }

    LOG_DEBUG0("_init_main_task: setting main task as current task");
    _ppos_core_set_current_task(ppos_core->main_task);

exit:
    LOG_TRACE("_init_main_task: exiting (%d)", ret);
    return ret;
}

int _init_dispatcher_task()
{
    LOG_TRACE0("_init_dispatcher_task: entered");
    int ret = _init_system_task(&ppos_core->dispatcher_task, TASK_TYPE_DISPATCHER, 
                               (void (*)(void *))dispatcher, &ppos_core->ready_queue);
    
    if (ret < PPOS_CORE_SUCCESS) {
        LOG_WARN("_init_dispatcher_task: failed to initialize dispatcher task");
        goto exit;
    }
    
    LOG_DEBUG0("_init_dispatcher_task: linking dispatcher task to main task");
    ppos_core->dispatcher_task->context.uc_link = &ppos_core->main_task->context;

exit:
    LOG_TRACE("_init_dispatcher_task: exiting (%d)", ret);
    return ret;
}

void _main_task_wrapper(void *arg)
{
    LOG_TRACE0("_main_task_wrapper: entered");
    
    int exit_code = (arg == NULL) ? 0 : *((int*)arg);
    LOG_DEBUG("_main_task_wrapper: calling exit with code %d", exit_code);
    
    exit(exit_code);
}

void _ppos_core_destroy()
{
    LOG_TRACE0("_ppos_core_destroy: entered");

    if (ppos_core->dispatcher_task != NULL) {
        _cleanup_task_stack(ppos_core->dispatcher_task);
        
        LOG_DEBUG0("_ppos_core_destroy: freeing dispatcher task");
        free(ppos_core->dispatcher_task);
        ppos_core->dispatcher_task = NULL;
    }

    if (ppos_core->main_task != NULL) {
        _cleanup_task_stack(ppos_core->main_task);
        
        LOG_DEBUG0("_ppos_core_destroy: freeing main task");
        free(ppos_core->main_task);
        ppos_core->main_task = NULL;
    }

    if (ppos_core != NULL) {
        LOG_DEBUG0("_ppos_core_destroy: freeing ppos_core");
        free(ppos_core);
        ppos_core = NULL;
    }

    LOG_TRACE0("_ppos_core_destroy: exiting (void)");
}

void _ppos_core_incr_task_cnt()
{
    LOG_TRACE0("_ppos_core_incr_task_cnt: entered");
    ppos_core->task_cnt = ppos_core->task_cnt + 1;
    LOG_TRACE("_ppos_core_incr_task_cnt: task_cnt incremented to %d", ppos_core->task_cnt);
    LOG_TRACE0("_ppos_core_incr_task_cnt: exiting (void)");
}

void _set_task_id(task_t *task)
{
    LOG_TRACE0("_set_task_id: entered");

    LOG_TRACE("_set_task_id: setting task id to %d", ppos_core->task_cnt);
    task->id = ppos_core->task_cnt;
    _ppos_core_incr_task_cnt();

    LOG_TRACE0("_set_task_id: exiting (void)");
}

void _ppos_core_set_current_task(task_t *task)
{
    LOG_TRACE0("_ppos_core_set_current_task: entered");
    LOG_TRACE("_ppos_core_set_current_task: setting current task to task %d", task->id);
    ppos_core->current_task = task;
    LOG_TRACE0("_ppos_core_set_current_task: exiting (void)");
}

void _setup_task_stack(task_t *task, stack_t *stack)
{
    LOG_TRACE0("_setup_task_stack: entered");

    LOG_DEBUG("_setup_task_stack: registering stack on valgrind");
    task->vg_id = VALGRIND_STACK_REGISTER(stack, stack + STACKSIZE);

    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;

    LOG_TRACE0("_setup_task_stack: exiting (void)");
}

void _link_task_to_dispatcher(task_t *task)
{
    LOG_TRACE0("_link_task_to_dispatcher: entered");

    LOG_DEBUG("_link_task_to_dispatcher: linking user task %d to dispatcher", task->id);
    task->context.uc_link = &ppos_core->dispatcher_task->context;

    LOG_TRACE0("_link_task_to_dispatcher: exiting (void)");
}

void _cleanup_task_stack(task_t *task)
{
    LOG_TRACE("_cleanup_task_stack: entered for task %d", task->id);
    
    if (task->context.uc_stack.ss_sp != NULL) {
        LOG_DEBUG("_cleanup_task_stack: freeing task %d stack", task->id);
        free(task->context.uc_stack.ss_sp);
        task->context.uc_stack.ss_sp = NULL;

        LOG_DEBUG("_cleanup_task_stack: unregistering task %d stack on valgrind", task->id);
        VALGRIND_STACK_DEREGISTER(task->vg_id);
    }
    
    LOG_TRACE("_cleanup_task_stack: exiting for task %d", task->id);
}
