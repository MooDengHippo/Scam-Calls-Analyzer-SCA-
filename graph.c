#include "graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "hash_map.h"

// ▸ Return existing node or allocate a new one
GraphNode *graph_get_node(GraphNode *nodes[], const char *phone) {
    for (int i = 0; i < MAX_NODES; ++i)
        if (nodes[i] && strcmp(nodes[i]->phone, phone) == 0)
            return nodes[i];

    // allocate new slot
    for (int i = 0; i < MAX_NODES; ++i)
        if (!nodes[i]) {
            nodes[i] = calloc(1, sizeof(GraphNode));
            strncpy(nodes[i]->phone, phone, MAX_PHONE_LENGTH);
            return nodes[i];
        }
    return NULL; // graph full
}

// ▸ Add bidirectional edge a—b (ignores duplicates & overflow)
void graph_add_edge(GraphNode *nodes[], const char *a, const char *b) {
    GraphNode *na = graph_get_node(nodes, a);
    GraphNode *nb = graph_get_node(nodes, b);
    if (!na || !nb) return;

    if (na->neighbor_count < MAX_NEIGHBORS)
        na->neighbors[na->neighbor_count++] = nb;
    if (nb->neighbor_count < MAX_NEIGHBORS)
        nb->neighbors[nb->neighbor_count++] = na;
}

// ▸ Breadth‑first traversal (prints reachable numbers)
void graph_bfs(GraphNode *nodes[], const char *start_phone) {
    for (int i = 0; i < MAX_NODES; ++i)
        if (nodes[i]) nodes[i]->visited = 0;

    GraphNode *start = graph_get_node(nodes, start_phone);
    if (!start) { printf("No data for %s\n", start_phone); return; }

    Queue *q = queue_init();
    enqueue(q, start); start->visited = 1;
    printf("BFS network for %s:\n", start_phone);

    while (!queue_is_empty(q)) {
        GraphNode *cur = dequeue(q);
        printf("  %s\n", cur->phone);
        for (int i = 0; i < cur->neighbor_count; ++i) {
            GraphNode *nbr = cur->neighbors[i];
            if (!nbr->visited) {
                nbr->visited = 1;
                enqueue(q, nbr);
            }
        }
    }
    queue_free(q);
}

// ▸ Depth‑first traversal helper
static void dfs_visit(GraphNode *node) {
    node->visited = 1;
    printf("  %s\n", node->phone);
    for (int i = 0; i < node->neighbor_count; ++i)
        if (!node->neighbors[i]->visited)
            dfs_visit(node->neighbors[i]);
}

void graph_dfs(GraphNode *nodes[], const char *start_phone) {
    for (int i = 0; i < MAX_NODES; ++i)
        if (nodes[i]) nodes[i]->visited = 0;
    GraphNode *start = graph_get_node(nodes, start_phone);
    if (!start) { printf("No data for %s\n", start_phone); return; }

    printf("DFS network for %s:\n", start_phone);
    dfs_visit(start);
}

void graph_node_free(GraphNode *node) {
    // ไม่มี malloc ซ้อน → แค่ล้างข้อมูลใน node ก็พอ
    // แต่ถ้าจะให้ปลอดภัย (เช่นป้องกัน double free) ให้ตรวจเช็คก่อน
    if (node) {
        node->neighbor_count = 0;
        node->visited = 0;
        for (int i = 0; i < MAX_NEIGHBORS; i++) {
            node->neighbors[i] = NULL;
        }
        free(node);
    }
}