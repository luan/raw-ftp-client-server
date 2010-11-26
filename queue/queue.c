#include "queue.h"
#include <stdio.h>

t_queue *queue_new() {
    t_queue *q = (t_queue *) malloc(sizeof(t_queue));
    t_message value;
    value.begin = 0;
    q->value = value;
    return q;
}

void enqueue(t_queue *q, const t_message value) {
    t_queue *e;
    for (e = q; e->next && e->value.begin != 0; e = e->next);
    e->next = queue_new();
    e->value = value;
}

t_message dequeue(t_queue **q) {
    t_message value;
    value.begin = 0;
    if (empty(*q)) return value;

    value = (*q)->value;
    t_queue *r = *q;
    *q = (*q)->next;
    free(r->value.data);
    free(r);
    return value;
}

int has_element(t_queue *q, unsigned char sequence) {
    t_queue *e;
    int i = 0;
    for (e = q; e->value.begin != 0; e = e->next, i++)
        if (e->value.sequence == sequence)
            return 1;
    return 0;
}

t_message get_element(t_queue *q, unsigned char sequence) {
    t_queue *e;
    t_message r;
    r.begin = 0;
    int i = 0;
    for (e = q; e->value.begin != 0; e = e->next, i++);
    if (e->value.sequence == sequence)
        return e->value;

    return r;
}

unsigned char dequeue_until(t_queue **q, unsigned char sequence) {
    if (queue_size(*q) < 1)
        return 0;
    unsigned char i = 0;

    t_message message;
    do {
        message = dequeue(q);
        i++;
    } while(sequence != message.sequence);

    return i;
}

int queue_size(t_queue *q) {
    t_queue *e;
    int i = 0;

    for (e = q; e && e->value.begin != 0; e = e->next, i++);
    return --i;
}

int empty(t_queue *q) {
    return (q->value.begin == 0);
}
