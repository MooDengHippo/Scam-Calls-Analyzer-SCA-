#ifndef HASH_MAP_H
#define HASH_MAP_H
#include "utils.h"
#include <stddef.h> // for size_t
#define TABLE_SIZE 2003 // Prime number hash table size for better distribution

// Record structure for scam data
typedef struct ScamRecord{

    char phone[MAX_PHONE_LENGTH]; // Normalized phone number
    float suspicious_score;       // Risk score (0–1)
    int report_count;             // Number of reports
    struct ScamRecord *next;      // Linked list for collision handling

}ScamRecord;

// Hash table mapping phone numbers to scam records
typedef struct{

    ScamRecord *buckets[TABLE_SIZE];

}HashMap;

// Function declarations
HashMap* hash_map_init(void);
void hash_map_insert(HashMap *map, const char *phone, float score, int reports);
ScamRecord* hash_map_lookup(HashMap *map, const char *phone);
void hash_map_free(HashMap *map);

#endif // HASH_MAP_H