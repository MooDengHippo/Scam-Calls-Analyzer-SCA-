#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "queue.h"

/*
 * Retrieve a node for the given phone number from the graph.
 * If it doesn't exist, create a new one.
 */
GraphNode *graph_get_node(GraphNode *nodes[], const char *phone){

    for(int i = 0; i < MAX_NODES; ++i)
        if(nodes[i] && strcmp(nodes[i]->phone, phone) == 0)
            return nodes[i];

    for(int i = 0; i < MAX_NODES; ++i)
        if(!nodes[i]){
            nodes[i] = calloc(1, sizeof(GraphNode));
            strncpy(nodes[i]->phone, phone, MAX_PHONE_LENGTH);
            nodes[i]->phone[MAX_PHONE_LENGTH - 1] = '\0';
            return nodes[i];
        }
    return NULL;

}
// Helper: Check if node a is already connected to node b
static int already_connected(GraphNode *a, GraphNode *b){

    for(int i = 0; i < a->neighbor_count; ++i){
        if(a->neighbors[i] == b) return 1;
    }
    return 0;
}
/*
 * Add a bidirectional edge between two phone numbers.
 * Prevents overflow if the neighbor list is full.
 */
void graph_add_edge(GraphNode *nodes[], const char *a, const char *b){

    GraphNode *na = graph_get_node(nodes, a);
    GraphNode *nb = graph_get_node(nodes, b);
    if(!na || !nb) return;

    if(!already_connected(na, nb) && na->neighbor_count < MAX_NEIGHBORS)
        na->neighbors[na->neighbor_count++] = nb;

    if(!already_connected(nb, na) && nb->neighbor_count < MAX_NEIGHBORS)
        nb->neighbors[nb->neighbor_count++] = na;

}
/*
 * Perform Breadth-First Search from a given start phone number.
 * Prints all reachable nodes.
 */
void graph_bfs(GraphNode *nodes[], const char *start_phone){

    for(int i = 0; i < MAX_NODES; ++i)
        if(nodes[i]) nodes[i]->visited = 0;

    GraphNode *start = graph_get_node(nodes, start_phone);
    if(!start){
        printf("Have no data for %s\n", start_phone);
        return;
    }

    Queue *q = queue_init();
    enqueue(q, start);
    start->visited = 1;
    printf("BFS network for %s:\n", start_phone);

    while(!queue_is_empty(q)){
        GraphNode *cur = dequeue(q);
        printf("  %s (Degree: %d)\n", cur->phone, cur->neighbor_count);
        for(int i = 0; i < cur->neighbor_count; ++i){
            GraphNode *nbr = cur->neighbors[i];
            if(!nbr->visited){
                nbr->visited = 1;
                enqueue(q, nbr);
            }
        }
    }
    queue_free(q);

}
// Helper for Depth-First Search
static void dfs_visit(GraphNode *node){

    node->visited = 1;
    printf("  %s\n", node->phone);
    for(int i = 0; i < node->neighbor_count; ++i)
        if(!node->neighbors[i]->visited)
            dfs_visit(node->neighbors[i]);

}
/*
 * Perform Depth-First Search from a given start phone number.
 * Prints all reachable nodes.
 */
void graph_dfs(GraphNode *nodes[], const char *start_phone){

    for(int i = 0; i < MAX_NODES; ++i)
        if(nodes[i]) nodes[i]->visited = 0;

    GraphNode *start = graph_get_node(nodes, start_phone);
    if(!start){
        printf("Have data for %s\n", start_phone);
        return;
    }

    printf("DFS network for %s:\n", start_phone);
    dfs_visit(start);

}
// Remove edge relation
void graph_remove_edge(GraphNode *nodes[], const char *phoneA, const char *phoneB){

    GraphNode *a = graph_get_node(nodes, phoneA);
    GraphNode *b = graph_get_node(nodes, phoneB);
    if(!a || !b) return;

    // Remove B from A's neighbors
    for(int i = 0; i < a->neighbor_count; ++i){
        if(strcmp(a->neighbors[i]->phone, phoneB) == 0){
            for(int j = i; j < a->neighbor_count - 1; ++j)
                a->neighbors[j] = a->neighbors[j + 1];
            a->neighbor_count--;
            break;
        }
    }

    // Remove A from B's neighbors
    for(int i = 0; i < b->neighbor_count; ++i){
        if(strcmp(b->neighbors[i]->phone, phoneA) == 0){
            for(int j = i; j < b->neighbor_count - 1; ++j)
                b->neighbors[j] = b->neighbors[j + 1];
            b->neighbor_count--;
            break;
        }
    }
}
/*
 * Frees a graph node safely. Clears neighbors and resets status.
 */
void graph_node_free(GraphNode *node){

    if(node){
        node->neighbor_count = 0;
        node->visited = 0;
        for(int i = 0; i < MAX_NEIGHBORS; ++i)
            node->neighbors[i] = NULL;
        free(node);
    }
    
}