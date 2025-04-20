#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "utils.h"
#include <stddef.h>

#define TABLE_SIZE 2003

typedef struct ScamRecord {
    char  phone[MAX_PHONE_LENGTH];
    float suspicious_score;
    int   report_count;
    struct ScamRecord *next;
} ScamRecord;

typedef struct {
    ScamRecord *buckets[TABLE_SIZE];
} HashMap;

// Initializes a new hash map and returns a pointer to it.
HashMap* hash_map_init(void);

// Inserts or updates a ScamRecord in the map
void hash_map_insert(HashMap *m, const char *phone, float score, int reports);

// Looks up a phone number. Returns ScamRecord* if found, or NULL if not found.
ScamRecord* hash_map_lookup(HashMap *m, const char *phone);

// Frees all memory used by the hash map
void hash_map_free(HashMap *m);

#endif // HASH_MAP_H