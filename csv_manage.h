#ifndef CSV_MANAGE_H
#define CSV_MANAGE_H
#include "hash_table.h"
#include "graph.h"
/*
 * Read data from CSV file and populate the system:
 * - Populates Hash Table with suspicious numbers
 * - Populates Graph with phone-to-phone relationships
 * - Format:
 *     R,<phone>,<suspicious_score 0 to 1>,<report_count>
 *     E,<phone1>,<phone2>
 * - Returns number of valid rows parsed, or -1 on error
 */
int csv_read_data(const char *fname, HashTable *map, GraphNode *nodes[]);
/*
 * Write the current state of the HashMap back to CSV
 * - Used on exit to persist the system state
 * - Format:
 *    R,<phone>,<score>,<report_count>
 */
int csv_write_data(const char *fname, HashTable *map);
/*
 * Append all graph edges (relationships) to CSV
 * - Appends after the scam record data
 * - Format:
 *    E,<phone1>,<phone2>
 */
int csv_write_edges(const char *fname, GraphNode **nodes);

#endif // CSV_MANAGE_H