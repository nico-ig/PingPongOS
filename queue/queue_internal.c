#include <stdio.h>

#include "queue.h"
#include "queue_internal.h"
#include "logger.h"

int _is_empty(queue_t *queue)
{
    LOG_TRACE0("_is_empty: entered");
    int is_empty = queue == NULL;
    LOG_TRACE("_is_empty: exiting (%d)", is_empty);
    return is_empty;
}

queue_status_t _validate_append_parameters(queue_t **queue, queue_t *elem)
{
    LOG_TRACE0("_validate_append_parameters: entered");

    queue_status_t ret;

    if (queue == NULL) {
        LOG_WARN0("_validate_append_parameters: queue pointer is NULL");
        ret = QUEUE_ERR_NULL_PTR;
        goto exit;
    }
    
    if (elem == NULL) {
        LOG_WARN0("_validate_append_parameters: element pointer is NULL");
        ret = QUEUE_ERR_ELEM_NOT_EXIST;
        goto exit;
    }

    if (elem->prev != NULL || elem->next != NULL) {
        LOG_WARN("_validate_append_parameters: element %p is already in a queue", (void*)elem);
        ret = QUEUE_ERR_ALREADY_IN_QUEUE;
        goto exit;
    }

    ret = QUEUE_OK;
    goto exit;

exit:
    LOG_TRACE("_validate_append_parameters: exiting (%d)", (int)ret);
    return ret;
}

queue_t *_link_element(queue_t *head, queue_t *elem, queue_t *tail)
{
    _log_pre_insertion_state(head, elem, tail);

    LOG_TRACE0("_link_element: entered");
    elem->prev = tail;
    elem->next = head;

    tail->next = elem;
    head->prev = elem;
    _log_post_insertion_state(head, elem);

    LOG_TRACE0("_link_element: exiting");
    return head;
}

void _log_pre_insertion_state(queue_t *head, queue_t *elem, queue_t *tail)
{
    (void)head;
    (void)elem;
    (void)tail;

    LOG_TRACE0("_log_pre_insertion_state: entered");
    LOG_TRACE("_log_pre_insertion_state: Pre-insertion state:");
    LOG_TRACE("_log_pre_insertion_state:  HEAD: %p (prev=%p)", (void*)head, (void*)head->prev);
    LOG_TRACE("_log_pre_insertion_state:  TAIL: %p (next=%p)", (void*)tail, (void*)tail->next);
    LOG_TRACE("_log_pre_insertion_state:  ELEM: %p (new tail)", (void*)elem);
    LOG_TRACE0("_log_pre_insertion_state: exiting (void)");
}

void _log_post_insertion_state(queue_t *head, queue_t *new_tail)
{
    (void)head;
    (void)new_tail;

    LOG_TRACE0("_log_post_insertion_state: entered");
    LOG_TRACE("_log_post_insertion_state: Post-insertion state:");
    LOG_TRACE("_log_post_insertion_state:  HEAD: %p (prev=%p)", (void*)head, (void*)head->prev);
    LOG_TRACE("_log_post_insertion_state:  TAIL: %p (prev=%p, next=%p)", (void*)new_tail, (void*)new_tail->prev, (void*)new_tail->next);
    LOG_TRACE0("_log_post_insertion_state: exiting (void)");
}

queue_status_t _validate_remove_parameters(queue_t **queue, queue_t *elem)
{
    LOG_TRACE0("_validate_remove_parameters: entered");

    queue_status_t ret;

    if (queue == NULL) {
        LOG_WARN0("_validate_remove_parameters: queue pointer is NULL");
        ret = QUEUE_ERR_NULL_PTR;
        goto exit;
    }
    
    if (elem == NULL) {
        LOG_WARN0("_validate_remove_parameters: element pointer is NULL");
        ret = QUEUE_ERR_ELEM_NOT_EXIST;
        goto exit;
    }

    ret = _find_element(*queue, elem);
    if (ret != QUEUE_OK) {
        LOG_WARN("_validate_remove_parameters: element %p not found in the queue %p", (void*)elem, (void*)queue);
        goto exit;
    }

    ret = QUEUE_OK;
    goto exit;

exit:
    LOG_TRACE("_validate_remove_parameters: exiting (%d)", (int)ret);
    return ret;
}

queue_status_t _find_element(queue_t *queue, queue_t *elem)
{
    LOG_TRACE0("_find_element: entered");

    queue_status_t ret;

    if (_is_empty(queue)) {
        LOG_WARN("_find_element: queue %p is empty", (void*)queue);
        ret = QUEUE_ERR_NULL_PTR;
        goto exit;
    }

    LOG_TRACE("_find_element: searching for element %p in queue %p", (void*)elem, (void*)queue);

    queue_t *current = queue;

    do {
        LOG_TRACE("_find_element: checking %p == %p", (void*)elem, (void*)current);
        if (current == elem) {
            LOG_DEBUG("_find_element: element %p found on queue %p", (void*)elem, (void*)queue);
            ret = QUEUE_OK;
            goto exit;
        }
        current = current->next;
    } while (current != queue);

    LOG_DEBUG("_find_element: element %p not found on queue %p", (void*)elem, (void*)queue);
    ret = QUEUE_ERR_NULL_PTR;
    goto exit;

exit:
    LOG_TRACE("_find_element: exiting (%d)", ret);
    return ret;
}

void _unlink_element(queue_t **queue, queue_t *elem)
{
    LOG_TRACE0("_unlink_element: entered");
    queue_t *prev_elem = elem->prev;
    queue_t *next_elem = elem->next;

    _log_pre_removal_state(prev_elem, elem, next_elem);

    if (_is_last_element(elem)) {
        LOG_TRACE("_unlink_element: removing last element %p from queue %p, queue will be empty", (void*)elem, (void*)queue);
        *queue = NULL;
    } else {
        prev_elem->next = next_elem;
        next_elem->prev = prev_elem;
        
        if (*queue == elem) {
            *queue = next_elem;
            LOG_TRACE("_unlink_element: updated head of the queue %p to next element %p", (void*)queue, (void*)next_elem);
        }
    }

    _log_post_removal_state(prev_elem, next_elem);
    LOG_TRACE0("_unlink_element: exiting (void)");
}

void _log_pre_removal_state(queue_t *prev_elem, queue_t* elem, queue_t *next_elem)
{
    (void)prev_elem;
    (void)elem;
    (void)next_elem;

    LOG_TRACE0("_log_pre_removal_state: entered");
    LOG_TRACE("_log_pre_removal_state: Removal state:");
    LOG_TRACE("_log_pre_removal_state:  PREV: %p (prev=%p, next=%p)", (void*)prev_elem, (void*)prev_elem->prev, (void*)prev_elem->next);
    LOG_TRACE("_log_pre_removal_state:  ELEM: %p (prev=%p, next=%p)", (void*)elem, (void*)elem->prev, (void*)elem->next);
    LOG_TRACE("_log_pre_removal_state:  NEXT: %p (prev=%p, next=%p)", (void*)next_elem, (void*)next_elem->prev, (void*)next_elem->next);
    LOG_TRACE0("_log_pre_removal_state: exiting (void)");
}

int _is_last_element(queue_t *elem)
{
    LOG_TRACE0("_is_last_element: entered");
    int is_last = elem->next == elem;
    LOG_TRACE("_is_last_element: exiting (%d)", is_last);
    return is_last;
}

void _log_post_removal_state(queue_t *prev_elem, queue_t *next_elem)
{
    (void)prev_elem;
    (void)next_elem;

    LOG_TRACE0("_log_post_removal_state: entered");
    LOG_TRACE("_log_post_removal_state: Removal state:");
    LOG_TRACE("_log_post_removal_state:  PREV: %p (prev=%p, next=%p)", (void*)prev_elem, (void*)prev_elem->prev, (void*)prev_elem->next);
    LOG_TRACE("_log_post_removal_state:  NEXT: %p (prev=%p, next=%p)", (void*)next_elem, (void*)next_elem->prev, (void*)next_elem->next);
    LOG_TRACE0("_log_post_removal_state: exiting (void)");
}

void _cleanup_removed_element(queue_t *elem)
{
    LOG_TRACE0("_cleanup_removed_element: entered");
    elem->prev = NULL;
    elem->next = NULL;
    LOG_TRACE0("_cleanup_removed_element: exiting (void)");
}
