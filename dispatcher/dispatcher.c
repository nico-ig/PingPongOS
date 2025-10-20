#include <stdlib.h>
#include <valgrind/valgrind.h>

#include "dispatcher.h"
#include "ppos_data.h"
#include "ppos.h"
#include "logger.h"

#define TASK_AGING_DECAY 1

static ppos_core_t *_core;

static task_t* _scheduler()
{
    task_t *queue_head = _core->ready_queue;
    task_t* priority_task = queue_head;

    LOG_TRACE("scheduler: starting with task %d (%d) as priority", priority_task->id, priority_task->dynamic_priority);

    int visited = 0;
    int queue_len = queue_size((queue_t*)_core->ready_queue);
    for (task_t *queue = queue_head->next; visited < queue_len; queue = queue->next) {
        LOG_TRACE("scheduler: checking task %d (%d)", queue->id, queue->dynamic_priority);

        task_t *task = queue;

        if (task->dynamic_priority < priority_task->dynamic_priority) {
            priority_task->dynamic_priority -= TASK_AGING_DECAY;
            priority_task = task;
        }
        else {
            task->dynamic_priority -= TASK_AGING_DECAY;
        }
        visited++;
    }

    LOG_INFO("scheduler: selected task %d with priority %d and quantum %d", priority_task->id, priority_task->dynamic_priority, priority_task->remaining_quantum);

    if (_core->remove_task_from_queue(priority_task, &_core->ready_queue) >= 0) {
        priority_task->dynamic_priority = priority_task->priority;
        priority_task->remaining_quantum = priority_task->quantum;
    }
    else
    {
        LOG_WARN("scheduler: failed to remove task %d from ready queue. Priority will not be reset", priority_task->id);
    }

    return priority_task;
}

static void _schedule_next_task() {
    LOG_DEBUG("schedule_next_task: ready queue size: %d", queue_size((queue_t*)_core->ready_queue));        
    task_t *next_task = _scheduler();

    _core->dispatcher_task->status = TASK_STATUS_SUSPENDED;
    _core->enable_task_switch();

    LOG_TRACE("schedule_next_task: running task %d", next_task->id);

    task_switch(next_task);
    
    _core->block_task_switch();
    _core->dispatcher_task->status = TASK_STATUS_RUNNING;

    LOG_TRACE("schedule_next_task: task %d returned", next_task->id);

    if (next_task->status == TASK_STATUS_RUNNING)
    {    
        LOG_WARN("schedule_next_task: task %d is still running, should not reach here", next_task->id);
    }
}

static void _wakeup_sleeping_tasks() {
    LOG_DEBUG("wakeup_sleeping_tasks: sleep queue size: %d", queue_size((queue_t*)_core->sleep_queue));
        
    int queue_len = queue_size((queue_t*)_core->sleep_queue);
    int visited = 0;
    unsigned int current_time = systime();

    for (task_t *task = _core->sleep_queue; visited < queue_len; task = task->next) {
        visited++;

        if (task->wakeup_time > current_time) {
            LOG_TRACE("wakeup_sleeping_tasks: task %d not ready to wake up (%d > %d)", task->id, task->wakeup_time, current_time);
            continue;
        }

        LOG_INFO("wakeup_sleeping_tasks: waking up task %d", task->id);
        task_awake(task, &_core->sleep_queue);
    }
}

void dispatcher(ppos_core_t *core)
{
    _core = core;

    while (true) {
        _core->block_task_switch();
        _core->dispatcher_task->status = TASK_STATUS_RUNNING;

        if (queue_size((queue_t*)_core->sleep_queue) == 0 && queue_size((queue_t*)_core->ready_queue) == 0) {
            break;
        }


        if (queue_size((queue_t*)_core->sleep_queue) > 0) {
            _wakeup_sleeping_tasks();
        }

        if (queue_size((queue_t*)_core->ready_queue) > 0) {
            _schedule_next_task();
        }
        
        _core->dispatcher_task->status = TASK_STATUS_SUSPENDED;
        _core->enable_task_switch();
    }
    
    LOG_INFO("dispatcher: no more tasks in sleep queue or ready queue, exiting");
    task_exit(0);
}
