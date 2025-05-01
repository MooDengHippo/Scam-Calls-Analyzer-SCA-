#ifndef CSV_MANAGE_H
#define CSV_MANAGE_H
#include "hash_table.h"
#include "graph.h"

/*
 * Read CSV and populate Hash Table and Graph structure
 * Parameters:
 *   record_file -> path to scam_numbers.csv (R,... entries)
 *   edge_file   -> path to scam_edges.csv (E,... entries)
 *   table       -> hash table to populate
 *   nodes       -> graph node array for relationships
 * Returns:
 *   Number of records loaded or -1 on error
 */
int csv_read_data(const char *record_file, const char *edge_file, HashTable *table, GraphNode *nodes[]);
/*
 * Write phone records from hash table to CSV
 * Format: R, <phone>, <score>, <report_count>
 */
int csv_write_data(const char *fname, HashTable *table);
/*
 * Write graph relationships to CSV
 * Format: E, <phoneA>, <phoneB>
 */
int csv_write_edges(const char *fname, GraphNode *nodes[]);

#endif // CSV_MANAGE_H