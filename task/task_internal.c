#include <stdlib.h>
#include <valgrind/valgrind.h>

#include "task_internal.h"
#include "ppos_data.h"
#include "logger.h"
#include "queue.h"
#include "ppos.h"

#define STACKSIZE 64*1024

extern ppos_core_t* ppos_core;

int _create_task_context_internal(task_t *task, void (*start_func)(void *), void *arg)
{
    LOG_TRACE0("_create_task_context_internal: entered");

    int ret = PPOS_CORE_SUCCESS;

    LOG_TRACE0("_create_task_context_internal: getting context");
    getcontext(&task->context);

    LOG_TRACE0("_create_task_context_internal: allocating stack");
    stack_t *stack = calloc(1, STACKSIZE);

    if (stack == NULL) {
        LOG_WARN("_create_task_context_internal: failed to allocate stack");
        ret = PPOS_CORE_ERR_NULL_PTR;
        goto exit;
    }

    LOG_DEBUG("_create_task_context_internal: setting up task stack");
    task->vg_id = VALGRIND_STACK_REGISTER(stack, stack + STACKSIZE);
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;

    LOG_DEBUG("_create_task_context_internal: making context");
    makecontext(&task->context, (void (*)(void))start_func, 1, arg);

exit:
    LOG_TRACE("_create_task_context_internal: exiting (%d)", ret);
    return ret;
}


int _handle_user_task_initialization_internal(task_t *task)
{
    LOG_TRACE0("_handle_user_task_initialization_internal: entered");
    
    task->type = TASK_TYPE_USER;
    LOG_TRACE0("_handle_user_task_initialization_internal: appending user task to ready queue");
    int ret = queue_append(&ppos_core->ready_queue, (queue_t*)task);

    if (ret < PPOS_CORE_SUCCESS) {
        LOG_WARN("_handle_user_task_initialization_internal: failed to append task %d to ready queue", 
                 task->id);
        goto exit;
    }

    task->context.uc_link = &ppos_core->dispatcher_task->context;
    ret = PPOS_CORE_SUCCESS;

exit:
    LOG_TRACE("_handle_user_task_initialization_internal: exiting (%d)", ret);
    return ret;
}


int _validate_context_switch_params_internal(task_t *task)
{
    LOG_TRACE0("_validate_context_switch_params_internal: entered");

    int ret = PPOS_CORE_SUCCESS;

    if (task == NULL) {
        LOG_WARN("_validate_context_switch_params_internal: task is NULL");
        ret = PPOS_CORE_ERR_NULL_PTR;
        goto exit;
    }

    if (ppos_core == NULL) {
        LOG_WARN("_validate_context_switch_params_internal: ppos_core is NULL");
        ret = PPOS_CORE_ERR_NULL_PTR;
        goto exit;
    }

    if (ppos_core->current_task == NULL) {
        LOG_WARN("_validate_context_switch_params_internal: current_task is NULL");
        ret = PPOS_CORE_ERR_NULL_PTR;
        goto exit;
    }

exit:
    LOG_TRACE("_validate_context_switch_params_internal: exiting (%d)", ret);
    return ret;
}


void _cleanup_terminated_task_internal(task_t *task)
{
    LOG_TRACE0("_cleanup_terminated_task_internal: entered");
    
    if (task->context.uc_stack.ss_sp != NULL) {
        LOG_DEBUG("_cleanup_terminated_task_internal: freeing task %d stack", task->id);
        free(task->context.uc_stack.ss_sp);
        task->context.uc_stack.ss_sp = NULL;
        
        LOG_DEBUG("_cleanup_terminated_task_internal: unregistering task %d stack on valgrind", task->id);
        VALGRIND_STACK_DEREGISTER(task->vg_id);
    }

    LOG_TRACE0("_cleanup_terminated_task_internal: exiting (void)");
}
