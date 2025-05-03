#ifndef GRAPH_H
#define GRAPH_H
#include "phone_format.h"
#define MAX_NODES 1000 // Maximum graph size
#define MAX_NEIGHBORS 10 // Limit of neighbors per node
/*
 * GraphNode
 * ----------------------------------------------------
 * Represents a phone number and its direct connections.
 */
typedef struct GraphNode{

    char phone[MAX_PHONE_LENGTH];               // Normalized phone number
    struct GraphNode *neighbors[MAX_NEIGHBORS]; // Adjacency list
    int neighbor_count;                         // Number of connected nodes
    int visited;                                // BFS/DFS visited flag

}GraphNode;
// Get or create a graph node for a phone number.
GraphNode *graph_get_node(GraphNode *nodes[], const char *phone);
// Add a bidirectional edge between two phone numbers.
void graph_add_edge(GraphNode *nodes[], const char *p1, const char *p2);
// Perform BFS traversal from a starting phone number.
void graph_bfs(GraphNode *nodes[], const char *start_phone);
// Perform DFS traversal from a starting phone number.
void graph_dfs(GraphNode *nodes[], const char *start_phone);
// Delete edge
void graph_remove_edge(GraphNode *nodes[], const char *phoneA, const char *phoneB);
// Free a graph node and clear its data.
void graph_node_free(GraphNode *node);

#endif // GRAPH_H