#include <stdlib.h>

#include "ppos.h"
#include "ppos_core_internal.h"
#include "ppos_data.h"
#include "logger.h"
#include "queue.h"
#include "task.h"
#include "dispatcher.h"

ppos_core_t* ppos_core = NULL;

void ppos_init ()
{
    LOG_TRACE0("ppos_init: entered");

    LOG_INFO0("ppos_init: initializing PingPongOS system");

    LOG_TRACE0("ppos_init: setting stdout to unbuffered");
    setvbuf(stdout, 0, _IONBF, 0);

    LOG_TRACE0("ppos_init: initializing ppos_core");
    if (_init_ppos_core() < PPOS_CORE_SUCCESS) {
        LOG_ERR("ppos_init: failed to initialize ppos_core");
        goto exit;
    }

    LOG_TRACE0("ppos_init: initializing main task");
    if (_init_main_task() < PPOS_CORE_SUCCESS) {
        LOG_ERR("ppos_init: failed to initialize main task");
        goto exit;
    }

    LOG_TRACE0("ppos_init: initializing dispatcher task");
    if (_init_dispatcher_task() < PPOS_CORE_SUCCESS) {
        LOG_ERR("ppos_init: failed to initialize dispatcher task");
        goto exit;
    }

exit:
    LOG_TRACE0("ppos_init: exiting (void)");
}

int task_init(task_t *task, void (*start_func)(void *), void *arg)
{
    LOG_TRACE0("task_init: entered");

    LOG_DEBUG("task_init: creating task context");
    int ret = task_create_context(task, start_func, arg);

    if (ret < PPOS_CORE_SUCCESS) {
        LOG_ERR("task_init: failed to create task context");
        goto exit;
    }

    LOG_DEBUG("task_init: setting task id");
    _set_task_id(task);

    task->status = TASK_STATUS_READY;
    LOG_DEBUG("task_init: new task with id %d created with status %d", task->id, task->status);

    if (task->type != TASK_TYPE_MAIN && task->type != TASK_TYPE_DISPATCHER) {
        task->type = TASK_TYPE_USER;
        LOG_TRACE("task_init: setting task %d type to %d", task->id, task->type);
    }

    ret = task->id;

    switch (task->type) {
        case TASK_TYPE_MAIN:
            LOG_INFO("task_init: main task %d created", task->id);
            LOG_TRACE0("task_init: main task not added to ready queue");
            break;

        case TASK_TYPE_DISPATCHER:
            LOG_INFO("task_init: dispatcher task %d created", task->id);
            LOG_TRACE0("task_init: dispatcher task not added to ready queue");
            break;

        default:
            LOG_INFO("task_init: task %d created", task->id);
            LOG_DEBUG("task_init: handling user task initialization");
            ret = task_handle_user_initialization(task);
            if (ret < PPOS_CORE_SUCCESS) {
                LOG_ERR("task_init: failed to handle user task initialization");
                goto exit;
            }
            break;
    }

exit:
    LOG_TRACE("task_init: exiting (%d)", ret);
    return ret;
}

int task_id ()
{
    LOG_TRACE0("task_id: entered");
    
    LOG_TRACE("task_id: getting current task ID");
    int ret = PPOS_CORE_SUCCESS;
    
    if (ppos_core == NULL) {
        LOG_WARN("task_id: ppos_core is NULL");
        ret = PPOS_CORE_ERR_NULL_PTR;
        goto exit;
    }
    
    if (ppos_core->current_task == NULL) {
        LOG_WARN("task_id: current_task is NULL");
        ret = PPOS_CORE_ERR_NULL_PTR;
        goto exit;
    }
    
    ret = ppos_core->current_task->id;
    LOG_DEBUG("task_id: current task ID is %d", ret);

exit:
    LOG_TRACE("task_id: exiting (%d)", ret);
    return ret;
}

void task_exit(int exit_code)
{
    LOG_TRACE0("task_exit: entered");

    LOG_INFO("task_exit: task %d exiting with code %d", ppos_core->current_task->id, exit_code);

    if (ppos_core->current_task->type != TASK_TYPE_DISPATCHER) {
        LOG_TRACE("task_exit: setting task %d status to terminated", ppos_core->current_task->id);
        ppos_core->current_task->status = TASK_STATUS_TERMINATED;

        LOG_DEBUG("task_exit: switching from task %d to dispatcher task", ppos_core->current_task->id);
        task_switch(ppos_core->dispatcher_task);

        goto exit;
    }

    _ppos_core_destroy();

exit:
    LOG_TRACE0("task_exit: exiting (void)");
    return;
}

int task_switch(task_t *task)
{
    LOG_TRACE("task_switch: entered with task %d", task->id);
    
    int ret = task_validate_context_switch_params(task);
    if (ret < PPOS_CORE_SUCCESS) {
        LOG_ERR("task_switch: failed to validate context switch parameters (%d)", ret);
        goto exit;
    }

    task_t *current_task = ppos_core->current_task;
    LOG_INFO("task_switch: switching from task %d to task %d", current_task->id, task->id);

    _ppos_core_set_current_task(task);
    ret = swapcontext(&current_task->context, &task->context);

    if (ret < 0) {
        LOG_ERR("task_switch: failed to switch context (%d)", ret);
        ret = PPOS_CORE_ERR_CNTEX;
        goto exit;
    }
    
    LOG_DEBUG("task_switch: context switched successfully");
    ret = PPOS_CORE_SUCCESS;

exit:
    LOG_TRACE("task_switch: exiting (%d)", ret);
    return ret;
}

void task_yield ()
{
    LOG_TRACE0("task_yield: entered");

    LOG_INFO("task_yield: yielding task %d", ppos_core->current_task->id);

    LOG_TRACE("task_yield: setting task %d status to ready", ppos_core->current_task->id);
    ppos_core->current_task->status = TASK_STATUS_READY;

    LOG_DEBUG("task_yield: appending current task %d to ready queue", ppos_core->current_task->id);
    int ret = queue_append(&ppos_core->ready_queue, (queue_t*)ppos_core->current_task);

    if (ret < 0) {
        LOG_WARN("task_yield: failed to append current task %d to ready queue", ppos_core->current_task->id);
    }
    
    LOG_DEBUG("task_yield: switching to dispatcher task");
    task_switch(ppos_core->dispatcher_task);
    
    LOG_TRACE0("task_yield: exiting (void)");
}

