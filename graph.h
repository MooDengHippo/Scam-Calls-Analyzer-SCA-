#ifndef GRAPH_H
#define GRAPH_H
#include "utils.h"
#define MAX_NODES 1000
#define MAX_NEIGHBORS 10

typedef struct GraphNode{ 
    char phone[MAX_PHONE_LENGTH]; 
    struct GraphNode *neighbors[MAX_NEIGHBORS]; 
    int neighbor_count; 
    int visited;
}GraphNode;

GraphNode *graph_get_node(GraphNode *nodes[],const char *phone);
void graph_add_edge(GraphNode *nodes[],const char *p1,const char *p2);
void graph_bfs(GraphNode *nodes[],const char *start_phone);
void graph_dfs(GraphNode *nodes[],const char *start_phone);
void graph_node_free(GraphNode *node);
#endif