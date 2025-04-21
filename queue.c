#include "queue.h"
#include <stdlib.h>

Queue *queue_init(void) {
    Queue *q = malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enqueue(Queue *q, GraphNode *node) {
    QueueNode *n = malloc(sizeof(QueueNode));
    n->data = node; n->next = NULL;
    if (!q->rear) q->front = q->rear = n;
    else { q->rear->next = n; q->rear = n; }
}

GraphNode *dequeue(Queue *q) {
    if (!q->front) return NULL;
    QueueNode *n = q->front;
    GraphNode *res = n->data;
    q->front = n->next;
    if (!q->front) q->rear = NULL;
    free(n);
    return res;
}

int queue_is_empty(Queue *q) { return q->front == NULL; }

void queue_free(Queue *q) {
    while (!queue_is_empty(q)) dequeue(q);
    free(q);
}