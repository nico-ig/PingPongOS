#include <stdio.h>

#include "queue.h"
#include "queue_internal.h"
#include "logger.h"

int queue_size (queue_t *queue)
{
    LOG_TRACE0("queue_size: entered");
    int size;
    
    if (_is_empty(queue)) {
        LOG_TRACE("queue_size: queue %p is NULL (empty)", (void*)queue);
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

void queue_print (char *iname, queue_t *queue, void print_elem (void*) )
{
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

int queue_append (queue_t **queue, queue_t *elem)
{
    LOG_TRACE0("queue_append: entered");

    queue_status_t ret = _validate_append_parameters(queue, elem);

    if (ret != QUEUE_OK) {
        LOG_WARN("queue_append: validation failed");
        goto exit;
    }

    LOG_DEBUG("queue_append: adding element %p to queue %p", (void*)elem, (void*)queue);

    queue_t* head;
    queue_t* tail;

    if (_is_empty(*queue)) {
        LOG_TRACE("queue_append: queue %p is empty, adding first element", (void*)queue);
        head = elem;
        tail = elem;
    } else {
        head = *queue;
        tail = head->prev;
    }
    
    LOG_TRACE("queue_append: linking element %p to queue %p", (void*)elem, (void*)queue);
    *queue = _link_element(head, elem, tail);

    ret = QUEUE_OK;
    goto exit;

exit:
    LOG_TRACE("queue_append: exiting (%d)", (int)ret);
    return (int)ret;
}

int queue_remove (queue_t **queue, queue_t *elem)
{
    LOG_TRACE0("queue_remove: entered");

    queue_status_t ret = _validate_remove_parameters(queue, elem);
    if (ret != QUEUE_OK) {
        LOG_WARN("queue_remove: validation failed");
        goto exit;
    }

    LOG_DEBUG("queue_remove: removing element %p from queue %p", (void*)elem, (void*)queue);
    _unlink_element(queue, elem);

    LOG_TRACE("queue_remove: cleaning up removed element %p", (void*)elem);
    _cleanup_removed_element(elem);
    
    LOG_TRACE("queue_remove: element %p removed successfully from queue %p", (void*)elem, (void*)queue);
    ret = QUEUE_OK;

exit:
    LOG_TRACE("queue_remove: exiting (%d)", (int)ret);
    return (int)ret;
}

