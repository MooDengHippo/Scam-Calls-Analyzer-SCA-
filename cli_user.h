#ifndef CLI_USER_H
#define CLI_USER_H
#include "hash_table.h"
#include "graph.h"

/*
 * User Mode Entry Point
 * ---------------------------------------------------------
 * Handles user-side CLI interaction:
 * - Phone number lookup with suspicious score visualization
 * - Scam network relationship display using graph traversal
 * - Option to report unknown numbers to the admin for review
 * Parameters:
 *   table -> Pointer to initialized HashTable
 *   nodes -> Graph node array for keep relationships
 */
void user_mode(HashTable *table, GraphNode *nodes[]);

#endif // CLI_USER_H