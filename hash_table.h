#ifndef HASH_TABLE_H
#define HASH_TABLE_H
#include <stddef.h> // for size_t
#include "utils.h"
#define TABLE_SIZE 2003 // Prime number hash table size for better distribution

// Record structure for scam data
typedef struct ScamRecord{

    char phone[MAX_PHONE_LENGTH]; // Normalized phone number
    float suspicious_score;       // Risk score (0–1)
    int report_count;             // Number of reports
    struct ScamRecord *next;      // Linked list for collision handling

}ScamRecord;

// Hash Table mapping phone numbers to scam records
typedef struct{

    ScamRecord *buckets[TABLE_SIZE];

}HashTable;

// Function declarations
HashTable* hash_table_init(void);
void hash_table_insert(HashTable *table, const char *phone, float score, int reports);
ScamRecord* hash_table_lookup(HashTable *table, const char *phone);
void hash_table_free(HashTable *table);

#endif // HASH_TABLE_H