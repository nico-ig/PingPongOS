#include <stdlib.h>
#include <valgrind/valgrind.h>

#include "dispatcher.h"
#include "ppos_data.h"
#include "ppos.h"
#include "logger.h"

task_t* _scheduler(queue_t *ready_queue)
{
    return (task_t*)ready_queue;
}

void _free_task_stack(task_t *task)
{
    if (task->context.uc_stack.ss_sp)
    {
        //free(task->context.uc_stack.ss_sp);
        VALGRIND_STACK_DEREGISTER(task->vg_id);
    }
}

void dispatcher(queue_t **ready_queue)
{
    LOG_DEBUG("dispatcher: starting with %d tasks", queue_size(*ready_queue));

    while (queue_size(*ready_queue) > 0) {
        LOG_DEBUG("dispatcher: queue size: %d", queue_size(*ready_queue));

        task_t *next_task = _scheduler(*ready_queue);

        if (queue_remove(ready_queue, (queue_t*)next_task) < 0) {
            LOG_WARN("dispatcher: failed to remove task %d from ready queue", next_task->id);
            continue;
        }
      
        if (next_task == NULL) {
            LOG_WARN("dispatcher: failed to get next task, continuing");
            continue;
        }
        
        LOG_INFO("dispatcher: running task %d", next_task->id);        
        task_switch(next_task);
        LOG_DEBUG("dispatcher: task %d returned", next_task->id);

        switch (next_task->status) {
            case TASK_STATUS_READY:
                LOG_DEBUG("dispatcher: task %d is ready", next_task->id);
                break;
    
            case TASK_STATUS_SUSPENDED:
                LOG_DEBUG("dispatcher: task %d is suspended", next_task->id);
                break;

            case TASK_STATUS_TERMINATED:
                LOG_DEBUG("dispatcher: task %d is terminated", next_task->id);
                _free_task_stack(next_task);
                break;
    
            default:
                LOG_WARN("dispatcher: task %d has unknown status %d", next_task->id, next_task->status);
                break;
        }
    }

    LOG_INFO("dispatcher: no more tasks in ready queue");

    task_exit(0);
}
