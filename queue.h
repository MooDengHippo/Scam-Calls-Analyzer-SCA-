#ifndef QUEUE_H
#define QUEUE_H
#include "graph.h"

typedef struct QueueNode{ 
    GraphNode *data; 
    struct QueueNode *next;
}QueueNode;

typedef struct{ QueueNode *front,*rear; }Queue;
Queue *queue_init(void); 
void enqueue(Queue *q,GraphNode *n); 
GraphNode *dequeue(Queue *q); 
int queue_is_empty(Queue *q); 
void queue_free(Queue *q);
#endif