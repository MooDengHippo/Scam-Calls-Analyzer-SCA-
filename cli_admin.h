#ifndef CLI_ADMIN_H
#define CLI_ADMIN_H
#include "hash_map.h"
#include "graph.h"
/*
 * Admin Mode
 * -------------------------
 * Handles admin-side CLI interaction:
 * - Add suspicious phone record
 * - Add relationship edge
 */
void admin_mode(HashMap *map, GraphNode *nodes[]);

#endif // CLI_ADMIN_H