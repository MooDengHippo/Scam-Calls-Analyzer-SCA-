#ifndef CSV_HANDLE_H
#define CSV_HANDLE_H
#include "hash_map.h"
#include "graph.h"

// Read CSV and populate map + graph. Format:
//  type, field1, field2, field3
//  R, <phone>, <score 0‑1>, <reports>          // Record row
//  E, <phoneA>, <phoneB>                       // Edge row
// Returns number of rows processed or –1 on error.
int csv_read_data(const char *fname, HashMap *map, GraphNode *nodes[]);
int csv_write_data(const char *fname, HashMap *map);
#endif