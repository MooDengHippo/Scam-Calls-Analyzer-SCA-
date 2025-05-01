#ifndef CLI_ADMIN_H
#define CLI_ADMIN_H
#include "hash_table.h"
#include "graph.h"
/*
 * Admin Mode Entry Point
 * ---------------------------------------------------------------------
 * Handles administrator-side CLI interaction:
 * - Add suspicious phone records
 * - Add relationship links between phone numbers
 * - View and accept pending reports submitted by users
 * - Analyze any phone number by checking both DB and relationship graph
 * Parameters:
 *   table -> Pointer to initialized HashTable
 *   nodes -> Graph node array for relationships
 */
void admin_mode(HashTable *table, GraphNode *nodes[]);

#endif // CLI_ADMIN_H