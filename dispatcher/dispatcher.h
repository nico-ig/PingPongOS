#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#include "ppos_data.h"
#include "queue.h"

/*
 * @brief Dispatcher function
 * @param queue: pointer to the queue to be dispatched
 * @return void
 */
void dispatcher(queue_t **queue);

#endif
