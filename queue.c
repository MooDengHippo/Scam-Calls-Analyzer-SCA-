#include <stdlib.h>
#include "queue.h"

/*
 * Queue Initialization
 * ------------------------------------
 * Allocates and returns an empty queue.
 */
Queue *queue_init(void){

    Queue *queue = malloc(sizeof(Queue));
    if(!queue) return NULL;
    queue->front = NULL;
    queue->rear = NULL;
    return queue;

}
/*
 * Enqueue
 * -----------------------------------------
 * Adds a GraphNode to the end of the queue.
 */
void enqueue(Queue *queue, GraphNode *node){

    if(!queue || !node) return;
    
    QueueNode *new_node = malloc(sizeof(QueueNode));
    if(!new_node) return;
    
    new_node->data = node;
    new_node->next = NULL;

    if(!queue->rear){
        queue->front = queue->rear = new_node;
    }else{
        queue->rear->next = new_node;
        queue->rear = new_node;
    }

}
/*
 * Dequeue
 * ----------------------------------------------------------
 * Removes the front GraphNode from the queue and returns it.
 * Returns NULL if the queue is empty.
 */
GraphNode *dequeue(Queue *queue){

    if(!queue || !queue->front) return NULL;

    QueueNode *tempo = queue->front;
    GraphNode *node = tempo->data;

    queue->front = tempo->next;
    if(!queue->front){
        queue->rear = NULL;
    }

    free(tempo);
    return node;

}
/*
 * Queue Is Empty
 * ---------------------------------------------
 * Returns 1 if the queue is empty, 0 otherwise.
 */
int queue_is_empty(Queue *queue){

    return queue && queue->front == NULL;

}
/*
 * Queue Free
 * -----------------------------------
 * Frees all memory used by the queue.
 */
void queue_free(Queue *queue){

    if(!queue) return;
    while(!queue_is_empty(queue)){
        dequeue(queue);
    }
    free(queue);

}