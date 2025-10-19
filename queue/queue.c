#include <stdio.h>

#include "queue.h"
#include "logger.h"


int _find_element(queue_t *queue, queue_t *elem)
{
    if (queue == NULL) return -1;

    queue_t *current = queue;

    do {
        if (current == elem) return 0;
        current = current->next;
    } while (current != queue);

    return -1;
}

int queue_size (queue_t *queue)
{
    if (queue == NULL) return 0;

    int size = 1;
    queue_t *elem = queue->next;

    while (elem != queue) {
        size++;
        elem = elem->next;
    }

    return size;
}

void queue_print (char *iname, queue_t *queue, void print_elem (void*) )
{
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
}

int queue_append (queue_t **queue, queue_t *elem)
{
    if (elem == NULL || elem->prev != NULL || elem->next != NULL) return -1;

    queue_t* head;
    queue_t* tail;

    if (*queue == NULL) {
        *queue = elem;
        head = elem;
        tail = elem;
    } else {
        head = *queue;
        tail = head->prev;
    }
    
    elem->prev = tail;
    elem->next = head;

    tail->next = elem;
    head->prev = elem;

    LOG_TRACE("queue_append: element %p added to queue %p", (void*)elem, (void*)(*queue));

    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem)
{
    if (queue == NULL || elem == NULL) return -1;
    if (_find_element(*queue, elem) != 0) return -1;

    LOG_TRACE("queue_remove: removing element %p from queue %p", (void*)elem, (void*)(*queue));

    queue_t *prev_elem = elem->prev;
    queue_t *next_elem = elem->next;

    if (elem->next == elem) {
        *queue = NULL;
    } else {
        prev_elem->next = next_elem;
        next_elem->prev = prev_elem;
        
        if (*queue == elem) *queue = next_elem;
    }

    elem->prev = NULL;
    elem->next = NULL;
    
    return 0;
}
