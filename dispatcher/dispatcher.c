#include <stdlib.h>
#include <valgrind/valgrind.h>

#include "dispatcher.h"
#include "dispatcher_internal.h"
#include "ppos_data.h"
#include "logger.h"
#include "queue.h"
#include "task.h"

void dispatcher(queue_t **ready_queue)
{
    LOG_TRACE0("dispatcher: entered");

    LOG_DEBUG("dispatcher: starting with %d tasks", queue_size(*ready_queue));

    LOG_TRACE0("dispatcher: starting dispatcher loop");
    while (queue_size(*ready_queue) > 0) {
        
        LOG_DEBUG("dispatcher: queue size: %d", queue_size(*ready_queue));
        task_t *next_task = _get_next_task(ready_queue);
        
        if (next_task == NULL) {
            LOG_WARN("dispatcher: failed to get next task, continuing");
            continue;
        }
        
        LOG_INFO("dispatcher: chosen task %d", next_task->id);
        task_execute(next_task);
        task_handle_status(next_task);
    }

    LOG_INFO("dispatcher: no more tasks in ready queue");

    LOG_TRACE0("dispatcher: exiting (void)");
    task_exit(0);
}
