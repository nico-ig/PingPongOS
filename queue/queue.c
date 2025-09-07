/*

Nome: Nico I G Ramos

GRR: 20210574

*/
#include <stdio.h>

#include "queue.h"
#include "logger.h"

typedef enum {
    QUEUE_OK = 0,
    QUEUE_ERR_NULL_PTR = -1,
    QUEUE_ERR_ELEM_NOT_EXIST = -2,
    QUEUE_ERR_ALREADY_IN_QUEUE = -3
} queue_status_t;

static int _is_empty(queue_t *queue);
static queue_status_t _validate_append_parameters(queue_t **queue, queue_t *elem);
static queue_t *_link_element(queue_t *head, queue_t *elem, queue_t *tail);
static void _log_pre_insertion_state(queue_t *head, queue_t *elem, queue_t *tail);
static void _log_post_insertion_state(queue_t *head, queue_t *new_tail);
static queue_status_t _validate_remove_parameters(queue_t **queue, queue_t *elem);
static queue_status_t _find_element(queue_t *queue, queue_t *elem);
static void _unlink_element(queue_t **queue, queue_t *elem);
static void _log_pre_removal_state(queue_t *prev_elem, queue_t* elem, queue_t *next_elem);
static int _is_last_element(queue_t *elem);
static void _log_post_removal_state(queue_t *prev_elem, queue_t *next_elem);
static void _cleanup_removed_element(queue_t *elem);

int queue_size (queue_t *queue) {
    LOG_TRACE0("queue_size: entered");
    int size;
    
    if (_is_empty(queue)) {
        LOG_DEBUG0("queue_size: queue is NULL (empty)");
        size = 0;
        goto exit;
    }

    size = 1;
    queue_t *elem = queue->next;

    while (elem != queue) {
        size++;
        elem = elem->next;
    }
    goto exit;

exit:
    LOG_TRACE("queue_size: exiting (%d)", size);
    return size;
}

void queue_print (char *iname, queue_t *queue, void print_elem (void*) ) {
    LOG_TRACE0("queue_print: entered");
    printf("%s[", iname);

    queue_t *elem = queue;
    int size = queue_size(queue);
    for (int i = 0; i < size - 1; i++) {
        print_elem(elem);
        elem = elem->next;
        printf(" ");
    }

    print_elem(elem);
    printf("]\n");
    LOG_TRACE0("queue_print: exiting (void)");
}

int queue_append (queue_t **queue, queue_t *elem) {
    LOG_TRACE0("queue_append: entered");

    queue_status_t ret = _validate_append_parameters(queue, elem);

    if (ret != QUEUE_OK) goto exit;

    queue_t* head;
    queue_t* tail;

    if (_is_empty(*queue)) {
        LOG_INFO0("queue_append: queue is empty, adding first element");
        head = elem;
        tail = elem;
    } else {
        head = *queue;
        tail = head->prev;
    }
    
    *queue = _link_element(head, elem, tail);

    ret = QUEUE_OK;
    goto exit;

exit:
    LOG_TRACE("queue_append: exiting (%d)", (int)ret);
    return (int)ret;
}

int queue_remove (queue_t **queue, queue_t *elem) {
    LOG_TRACE0("queue_remove: entered");

    queue_status_t ret = _validate_remove_parameters(queue, elem);
    if (ret != QUEUE_OK) goto exit;

    _unlink_element(queue, elem);
    _cleanup_removed_element(elem);
    
    LOG_INFO0("queue_remove: element removed successfully");
    ret = QUEUE_OK;

exit:
    LOG_TRACE("queue_remove: exiting (%d)", (int)ret);
    return (int)ret;
}

static int _is_empty(queue_t *queue) {
    LOG_TRACE0("_is_empty: entered");
    int is_empty = queue == NULL;
    LOG_TRACE("_is_empty: exiting (%d)", is_empty);
    return is_empty;
}

static queue_status_t _validate_append_parameters(queue_t **queue, queue_t *elem) {
    LOG_TRACE0("_validate_append_parameters: entered");

    queue_status_t ret;

    if (queue == NULL) {
        LOG_ERR0("_validate_append_parameters: queue pointer is NULL");
        ret = QUEUE_ERR_NULL_PTR;
        goto exit;
    }
    
    if (elem == NULL) {
        LOG_ERR0("_validate_append_parameters: element pointer is NULL");
        ret = QUEUE_ERR_ELEM_NOT_EXIST;
        goto exit;
    }

    if (elem->prev != NULL || elem->next != NULL) {
        LOG_ERR0("_validate_append_parameters: element is already in a queue");
        ret = QUEUE_ERR_ALREADY_IN_QUEUE;
        goto exit;
    }

    ret = QUEUE_OK;
    goto exit;

exit:
    LOG_TRACE("_validate_append_parameters: exiting (%d)", (int)ret);
    return ret;
}

static queue_t *_link_element(queue_t *head, queue_t *elem, queue_t *tail) {
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

static void _log_pre_insertion_state(queue_t *head, queue_t *elem, queue_t *tail) {
    (void)head;
    (void)elem;
    (void)tail;

    LOG_DEBUG0("_log_pre_insertion_state: entered");
    LOG_DEBUG0("_log_pre_insertion_state: Pre-insertion state:");
    LOG_DEBUG("_log_pre_insertion_state:  HEAD: %p (prev=%p)", (void*)head, (void*)head->prev);
    LOG_DEBUG("_log_pre_insertion_state:  TAIL: %p (next=%p)", (void*)tail, (void*)tail->next);
    LOG_DEBUG("_log_pre_insertion_state:  ELEM: %p (new tail)", (void*)elem);
    LOG_DEBUG0("_log_pre_insertion_state: exiting (void)");
}

static void _log_post_insertion_state(queue_t *head, queue_t *new_tail) {
    (void)head;
    (void)new_tail;

    LOG_DEBUG0("_log_post_insertion_state: entered");
    LOG_DEBUG0("_log_post_insertion_state: Post-insertion state:");
    LOG_DEBUG("_log_post_insertion_state:  HEAD: %p (prev=%p)", (void*)head, (void*)head->prev);
    LOG_DEBUG("_log_post_insertion_state:  TAIL: %p (prev=%p, next=%p)", (void*)new_tail, (void*)new_tail->prev, (void*)new_tail->next);
    LOG_DEBUG0("_log_post_insertion_state: exiting (void)");
}

static queue_status_t _validate_remove_parameters(queue_t **queue, queue_t *elem) {
    LOG_TRACE0("_validate_remove_parameters: entered");

    queue_status_t ret;

    if (queue == NULL) {
        LOG_ERR0("_validate_remove_parameters: queue pointer is NULL");
        ret = QUEUE_ERR_NULL_PTR;
        goto exit;
    }
    
    if (elem == NULL) {
        LOG_ERR0("_validate_remove_parameters: element pointer is NULL");
        ret = QUEUE_ERR_ELEM_NOT_EXIST;
        goto exit;
    }

    ret = _find_element(*queue, elem);
    if (ret != QUEUE_OK) {
        LOG_ERR0("_validate_remove_parameters: element not found in the queue");
        goto exit;
    }

    ret = QUEUE_OK;
    goto exit;

exit:
    LOG_TRACE("_validate_remove_parameters: exiting (%d)", (int)ret);
    return ret;
}

static queue_status_t _find_element(queue_t *queue, queue_t *elem) {
    LOG_TRACE0("_find_element: entered");

    queue_status_t ret;

    if (_is_empty(queue)) {
        LOG_ERR0("_find_element: queue is empty");
        ret = QUEUE_ERR_NULL_PTR;
        goto exit;
    }

    LOG_TRACE("_find_element: searching for element %p in queue %p", (void*)elem, (void*)queue);

    queue_t *current = queue;

    do {
        LOG_TRACE("_find_element: checking %p == %p", (void*)elem, (void*)current);
        if (current == elem) {
            LOG_DEBUG("_find_element: element %p found", (void*)elem);
            ret = QUEUE_OK;
            goto exit;
        }
        current = current->next;
    } while (current != queue);

    LOG_DEBUG0("_find_element: element not found");
    ret = QUEUE_ERR_NULL_PTR;
    goto exit;

exit:
    LOG_TRACE("_find_element: exiting (%d)", ret);
    return ret;
}

static void _unlink_element(queue_t **queue, queue_t *elem) {
    LOG_TRACE0("_unlink_element: entered");
    queue_t *prev_elem = elem->prev;
    queue_t *next_elem = elem->next;

    _log_pre_removal_state(prev_elem, elem, next_elem);

    if (_is_last_element(elem)) {
        LOG_INFO0("_unlink_element: removing last element, queue will be empty");
        *queue = NULL;
    } else {
        prev_elem->next = next_elem;
        next_elem->prev = prev_elem;
        
        if (*queue == elem) {
            *queue = next_elem;
            LOG_INFO0("_unlink_element: updated head of the queue to next element");
        }
    }

    _log_post_removal_state(prev_elem, next_elem);
    LOG_TRACE0("_unlink_element: exiting (void)");
}

static void _log_pre_removal_state(queue_t *prev_elem, queue_t* elem, queue_t *next_elem) {
    (void)prev_elem;
    (void)elem;
    (void)next_elem;

    LOG_DEBUG0("_log_pre_removal_state: entered");
    LOG_DEBUG0("_log_pre_removal_state: Removal state:");
    LOG_DEBUG("_log_pre_removal_state:  PREV: %p (prev=%p, next=%p)", (void*)prev_elem, (void*)prev_elem->prev, (void*)prev_elem->next);
    LOG_DEBUG("_log_pre_removal_state:  ELEM: %p (prev=%p, next=%p)", (void*)elem, (void*)elem->prev, (void*)elem->next);
    LOG_DEBUG("_log_pre_removal_state:  NEXT: %p (prev=%p, next=%p)", (void*)next_elem, (void*)next_elem->prev, (void*)next_elem->next);
    LOG_DEBUG0("_log_pre_removal_state: exiting (void)");
}

static int _is_last_element(queue_t *elem) {
    LOG_TRACE0("_is_last_element: entered");
    int is_last = elem->next == elem;
    LOG_TRACE("_is_last_element: exiting (%d)", is_last);
    return is_last;
}

static void _log_post_removal_state(queue_t *prev_elem, queue_t *next_elem) {
    (void)prev_elem;
    (void)next_elem;

    LOG_DEBUG0("_log_post_removal_state: entered");
    LOG_DEBUG0("_log_post_removal_state: Removal state:");
    LOG_DEBUG("_log_post_removal_state:  PREV: %p (prev=%p, next=%p)", (void*)prev_elem, (void*)prev_elem->prev, (void*)prev_elem->next);
    LOG_DEBUG("_log_post_removal_state:  NEXT: %p (prev=%p, next=%p)", (void*)next_elem, (void*)next_elem->prev, (void*)next_elem->next);
    LOG_DEBUG0("_log_post_removal_state: exiting (void)");
}

static void _cleanup_removed_element(queue_t *elem) {
    LOG_TRACE0("_cleanup_removed_element: entered");
    elem->prev = NULL;
    elem->next = NULL;
    LOG_TRACE0("_cleanup_removed_element: exiting (void)");
}
