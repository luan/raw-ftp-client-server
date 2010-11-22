#ifndef _QUEUE_
    #define _QUEUE_ value
#include <string.h>
#include <stdlib.h>
#include "../protocol/protocol.h"

typedef struct q_element {
    t_message value;
    struct q_element *next;
} t_queue;

t_queue *queue_new();
void enqueue(t_queue *q, const t_message value);
t_message dequeue(t_queue **q);
t_message dequeue_until(t_queue **q, unsigned char sequence);
int has_element(t_queue *q, unsigned char sequence);
int queue_size(t_queue *q);
int empty(t_queue *q);
#endif
