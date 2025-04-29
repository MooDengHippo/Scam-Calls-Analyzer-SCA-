#ifndef QUEUE_H
#define QUEUE_H
#include "graph.h"
// Queue node for holding GraphNode pointers
typedef struct QueueNode{

    GraphNode *data;
    struct QueueNode *next;

}QueueNode;

// Main queue structure with front and rear pointers
typedef struct Queue{

    QueueNode *front;
    QueueNode *rear;
    
}Queue;

// Initialize a new empty queue
Queue *queue_init(void);

// Add a node to the rear of the queue
void enqueue(Queue *queue, GraphNode *node);

// Remove and return the front node of the queue
GraphNode *dequeue(Queue *queue);

// Check if the queue is empty
int queue_is_empty(Queue *queue);

// Free all memory associated with the queue
void queue_free(Queue *queue);

#endif // QUEUE_H 