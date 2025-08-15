/*

Nome: Nico I G Ramos

GRR: 20210574

*/

#include "queue.h"

#ifdef LOG
#include "logger.h"
#else
#define LOG_ERR(fmt, ...) ;
#define LOG_WARN(fmt, ...) ;
#define LOG_INFO(fmt, ...)  ;
#define LOG_DEBUG(fmt, ...) ;
#define LOG_TRACE(fmt, ...) ;
#endif

typedef enum {
    QUEUE_OK = 0,
    QUEUE_ERR_NULL_PTR = -1,
    QUEUE_ERR_ELEM_NOT_EXIST = -2,
    QUEUE_ERR_ALREADY_IN_QUEUE = -3
} queue_status_t;

static int _is_first_element(queue_t *queue);
static queue_status_t _validate_append_parameters(queue_t **queue, queue_t *elem);
static queue_t *_link_element(queue_t *head, queue_t *elem, queue_t *tail);
static void _log_pre_insertion_state(queue_t *head, queue_t *elem, queue_t *tail);
static void _log_post_insertion_state(queue_t *head, queue_t *new_tail);

int queue_size (queue_t *queue) {
    LOG_TRACE("queue_size: entered");
    int size;
    
    if (_is_first_element(queue)) {
        LOG_DEBUG("queue_size: queue is NULL (empty)");
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

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
}

int queue_append (queue_t **queue, queue_t *elem) {
    LOG_TRACE("queue_append: entered");

    queue_status_t ret = _validate_append_parameters(queue, elem);

    if (ret != QUEUE_OK) goto exit;

    queue_t* head;
    queue_t* tail;

    if (_is_first_element(*queue)) {
        LOG_TRACE("queue_append: queue is empty, adding first element");
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
    return 0;
}

static int _is_first_element(queue_t *queue) {
    LOG_TRACE("_is_first_element: entered");
    int is_first = queue == NULL;   
    LOG_TRACE("_is_first_element: exiting (%d)", is_first);
    return is_first;
}

static queue_status_t _validate_append_parameters(queue_t **queue, queue_t *elem) {
    LOG_TRACE("_validate_append_parameters: entered");

    queue_status_t ret = QUEUE_OK;

    if (queue == NULL) {
        LOG_ERR("_validate_append_parameters: queue pointer is NULL");
        ret = QUEUE_ERR_NULL_PTR;
        goto exit;
    }
    
    if (elem == NULL) {
        LOG_ERR("_validate_append_parameters: element pointer is NULL");
        ret = QUEUE_ERR_ELEM_NOT_EXIST;
        goto exit;
    }

    if (elem->prev != NULL || elem->next != NULL) {
        LOG_ERR("_validate_append_parameters: element is already in a queue");
        ret = QUEUE_ERR_ALREADY_IN_QUEUE;
        goto exit;
    }

exit:
    LOG_TRACE("_validate_append_parameters: exiting (%d)", (int)ret);
    return ret;
}

static queue_t *_link_element(queue_t *head, queue_t *elem, queue_t *tail) {
    _log_pre_insertion_state(head, elem, tail);

    LOG_TRACE("_link_element: entered");
    elem->prev = tail;
    elem->next = head;

    tail->next = elem;
    head->prev = elem;
    _log_post_insertion_state(head, elem);

    LOG_TRACE("_link_element: exiting");
    return head;
}

static void _log_pre_insertion_state(queue_t *head, queue_t *elem, queue_t *tail) {
    LOG_TRACE("_log_pre_insertion_state: entered");
    LOG_TRACE("_log_pre_insertion_state: Pre-insertion state:");
    LOG_TRACE("_log_pre_insertion_state:  HEAD: %p (prev=%p)", (void*)head, (void*)head->prev);
    LOG_TRACE("_log_pre_insertion_state:  TAIL: %p (next=%p)", (void*)tail, (void*)tail->next);
    LOG_TRACE("_log_pre_insertion_state:  ELEM: %p (new tail)", (void*)elem);
    LOG_TRACE("_log_pre_insertion_state: exiting (void)");
}

static void _log_post_insertion_state(queue_t *head, queue_t *new_tail) {
    LOG_TRACE("_log_post_insertion_state: entered");
    LOG_TRACE("_log_post_insertion_state: Post-insertion state:");
    LOG_TRACE("_log_post_insertion_state:  HEAD: %p (prev=%p)", (void*)head, (void*)head->prev);
    LOG_TRACE("_log_post_insertion_state:  TAIL: %p (prev=%p, next=%p)", (void*)new_tail, (void*)new_tail->prev, (void*)new_tail->next);
    LOG_TRACE("_log_post_insertion_state: exiting (void)");
}
