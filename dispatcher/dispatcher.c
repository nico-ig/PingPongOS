#include <stdlib.h>
#include <valgrind/valgrind.h>

#include "dispatcher.h"
#include "ppos_data.h"
#include "ppos.h"
#include "logger.h"

#define TASK_AGING_DECAY 1

static task_t* _scheduler(queue_t **ready_queue)
{
    queue_t *queue_head = *ready_queue;
    task_t* priority_task = (task_t*)queue_head;

    LOG_TRACE("scheduler: starting with task %d (%d) as priority", priority_task->id, priority_task->dynamic_priority);

    for (queue_t *queue = queue_head->next; queue != queue_head; queue = queue->next) {
        LOG_TRACE("scheduler: checking task %d (%d)", ((task_t*)queue)->id, ((task_t*)queue)->dynamic_priority);

        task_t *task = (task_t*)queue;

        if (task->dynamic_priority < priority_task->dynamic_priority) {
            priority_task->dynamic_priority -= TASK_AGING_DECAY;
            priority_task = task;
        }
        else {
            task->dynamic_priority -= TASK_AGING_DECAY;
        }
    }

    LOG_INFO("scheduler: selected task %d with priority %d and quantum %d", priority_task->id, priority_task->dynamic_priority, priority_task->remaining_quantum);

    if (queue_remove(ready_queue, (queue_t*)priority_task) >= 0) {
        priority_task->dynamic_priority = priority_task->priority;
        priority_task->remaining_quantum = priority_task->quantum;
    }
    else
    {
        LOG_WARN("scheduler: failed to remove task %d from ready queue. Priority will not be reset", priority_task->id);
    }

    return priority_task;
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

        task_t *next_task = _scheduler(ready_queue);      
        LOG_TRACE("dispatcher: running task %d", next_task->id);
        
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
