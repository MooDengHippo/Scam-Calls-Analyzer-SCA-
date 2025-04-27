#ifndef CLI_USER_H
#define CLI_USER_H
#include "hash_map.h"
#include "graph.h"
/*
 * User Mode
 * -------------------------
 * Handles user-side CLI interaction:
 * - Phone number lookup
 * - Suspicious score visualization
 * - Scam network relationship display
 */
void user_mode(HashMap *map, GraphNode *nodes[]);

#endif // CLI_USER_H