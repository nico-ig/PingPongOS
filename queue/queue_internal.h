#ifndef __QUEUE_INTERNAL_H__
#define __QUEUE_INTERNAL_H__

#include "queue.h"

/*
 * @brief Queue status codes
 */
typedef enum {
    QUEUE_OK = 0,
    QUEUE_ERR_NULL_PTR = -1,
    QUEUE_ERR_ELEM_NOT_EXIST = -2,
    QUEUE_ERR_ALREADY_IN_QUEUE = -3
} queue_status_t;

/*
* @brief Check if the queue is empty
* @param queue: pointer to the queue to be checked
* @return int: 1 if the queue is empty, 0 otherwise
*/
int _is_empty(queue_t *queue);

/*
* @brief Validate the append parameters
* @param queue: pointer to the queue to be checked
* @param elem: pointer to the element to be appended
* @return queue_status_t: status of the operation
*/
queue_status_t _validate_append_parameters(queue_t **queue, queue_t *elem);

/*
* @brief Validate the remove parameters
* @param queue: pointer to the queue to be checked
* @param elem: pointer to the element to be removed
* @return queue_status_t: status of the operation
*/
queue_status_t _validate_remove_parameters(queue_t **queue, queue_t *elem);

/*
* @brief Find the element in the queue
* @param queue: pointer to the queue to be checked
* @param elem: pointer to the element to be found
* @return queue_status_t: status of the operation
*/
queue_status_t _find_element(queue_t *queue, queue_t *elem);

/*
* @brief Link the element to the queue
* @param head: pointer to the head of the queue
* @param elem: pointer to the element to be linked
* @param tail: pointer to the tail of the queue
* @return queue_t*: pointer to the head of the queue
*/
queue_t *_link_element(queue_t *head, queue_t *elem, queue_t *tail);

/*
* @brief Unlink the element from the queue
* @param queue: pointer to the queue to be checked
* @param elem: pointer to the element to be unlinked
* @return void
*/
void _unlink_element(queue_t **queue, queue_t *elem);

/*
* @brief Check if the element is the last element in the queue
* @param elem: pointer to the element to be checked
* @return int: 1 if the element is the last element, 0 otherwise
*/
int _is_last_element(queue_t *elem);

/*
* @brief Cleanup the removed element
* @param elem: pointer to the element to be cleaned up
* @return void
*/
void _cleanup_removed_element(queue_t *elem);

/*
* @brief Log the pre-insertion state of the queue
* @param head: pointer to the head of the queue
* @param elem: pointer to the element to be inserted
* @param tail: pointer to the tail of the queue
* @return void
*/
void _log_pre_insertion_state(queue_t *head, queue_t *elem, queue_t *tail);

/*
* @brief Log the post-insertion state of the queue
* @param head: pointer to the head of the queue
* @param new_tail: pointer to the new tail of the queue
* @return void
*/
void _log_post_insertion_state(queue_t *head, queue_t *new_tail);

/*
* @brief Log the pre-removal state of the queue
* @param prev_elem: pointer to the previous element of the queue
* @param elem: pointer to the element to be removed
* @param next_elem: pointer to the next element of the queue
* @return void
*/
void _log_pre_removal_state(queue_t *prev_elem, queue_t* elem, queue_t *next_elem);

/*
* @brief Log the post-removal state of the queue
* @param prev_elem: pointer to the previous element of the queue
* @param next_elem: pointer to the next element of the queue
* @return void
*/
void _log_post_removal_state(queue_t *prev_elem, queue_t *next_elem);

#endif
