#include <stdlib.h>
#include <valgrind/valgrind.h>

#include "task.h"
#include "task_internal.h"
#include "ppos_core_internal.h"
#include "ppos_data.h"
#include "logger.h"
#include "queue.h"
#include "ppos.h"

extern ppos_core_t* ppos_core;

int task_create_context(task_t *task, void (*start_func)(void *), void *arg)
{
    LOG_TRACE0("task_create_context: entered");

    int ret = _create_task_context_internal(task, start_func, arg);
    if (ret < PPOS_CORE_SUCCESS) {
        LOG_WARN("task_create_context: failed to create task context");
        goto exit;
    }

    ret = PPOS_CORE_SUCCESS;

exit:
    LOG_TRACE("task_create_context: exiting (%d)", ret);
    return ret;
}

int task_handle_user_initialization(task_t *task)
{
    LOG_TRACE0("task_handle_user_initialization: entered");
    
    int ret = _handle_user_task_initialization_internal(task);
    if (ret < PPOS_CORE_SUCCESS) {
        LOG_WARN("task_handle_user_initialization: failed to initialize user task");
        goto exit;
    }

    ret = PPOS_CORE_SUCCESS;

exit:
    LOG_TRACE("task_handle_user_initialization: exiting (%d)", ret);
    return ret;
}

int task_validate_context_switch_params(task_t *task)
{
    LOG_TRACE0("task_validate_context_switch_params: entered");

    int ret = _validate_context_switch_params_internal(task);
    if (ret < PPOS_CORE_SUCCESS) {
        LOG_WARN("task_validate_context_switch_params: validation failed");
        goto exit;
    }

    ret = PPOS_CORE_SUCCESS;

exit:
    LOG_TRACE("task_validate_context_switch_params: exiting (%d)", ret);
    return ret;
}

void task_execute(task_t *task)
{
    LOG_TRACE0("task_execute: entered");
    
    LOG_DEBUG("task_execute: switching to task %d", task->id);
    task_switch(task);
    LOG_DEBUG("task_execute: task %d returned", task->id);
    
    LOG_TRACE0("task_execute: exiting (void)");
}

void task_handle_status(task_t *task)
{
    LOG_TRACE0("task_handle_status: entered");
    
    switch (task->status) {
        case TASK_STATUS_READY:
            LOG_DEBUG("task_handle_status: task %d is ready", task->id);
            break;

        case TASK_STATUS_TERMINATED:
            LOG_DEBUG("task_handle_status: task %d is terminated", task->id);
            task_cleanup_terminated(task);
            break;

        default:
            LOG_WARN("task_handle_status: task %d has unknown status %d", task->id, task->status);
            task_cleanup_terminated(task);
            break;
    }
    
    LOG_TRACE0("task_handle_status: exiting (void)");
}

void task_cleanup_terminated(task_t *task)
{
    LOG_TRACE0("task_cleanup_terminated: entered");
    
    _cleanup_terminated_task_internal(task);

    LOG_TRACE0("task_cleanup_terminated: exiting (void)");
}
