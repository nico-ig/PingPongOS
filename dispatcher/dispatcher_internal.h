#ifndef __DISPATCHER_INTERNAL_H__
#define __DISPATCHER_INTERNAL_H__

#include "ppos.h"
#include "queue.h"

/*
* @brief Get the next task from the ready queue
* @param ready_queue: pointer to the queue to be dispatched
* @return pointer to the next task
*/
task_t* _get_next_task(queue_t **ready_queue);

/*
* @brief Scheduler function
* @param ready_queue: pointer to the queue to be dispatched
* @return pointer to the next task
*/
task_t* _scheduler(queue_t *ready_queue);

#endif
