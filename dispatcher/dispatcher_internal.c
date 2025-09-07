#include <stdlib.h>
#include <valgrind/valgrind.h>

#include "dispatcher_internal.h"
#include "logger.h"
#include "queue.h"
#include "ppos.h"

task_t* _get_next_task(queue_t **ready_queue)
{
    LOG_TRACE0("_get_next_task: entered");
    
    LOG_DEBUG("_get_next_task: getting next task, queue size: %d", queue_size(*ready_queue));
    task_t *next_task = _scheduler(*ready_queue);
    
    if (next_task == NULL) {
        LOG_WARN("_get_next_task: scheduler returned NULL task");
        goto exit;
    }
    
    LOG_DEBUG("_get_next_task: removing task %d from ready queue", next_task->id);
    int ret = queue_remove(ready_queue, (queue_t*)next_task);
    
    if (ret < 0) {
        LOG_WARN("_get_next_task: failed to remove task %d from ready queue", next_task->id);
        goto exit;
    }
    
    next_task->status = TASK_STATUS_RUNNING;

exit:
    LOG_TRACE("_get_next_task: exiting (%p)", (void*)next_task);
    return next_task;
}

task_t* _scheduler(queue_t *ready_queue)
{
    LOG_TRACE0("scheduler: entered");
    
    LOG_TRACE("scheduler: getting next task");
    task_t *next_task = (task_t*)ready_queue;

    LOG_TRACE("scheduler: exiting (%p)", (void*)next_task);
    return next_task;
}

