#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

/*
 * Hash Function
 * ------------------------------------------------
 * Uses basic polynomial rolling hash: h = h*31 + c
 * Returns index into the table (mod TABLE_SIZE)
 */
static unsigned int hash_function(const char *key){

    unsigned int h = 0;
    while(*key){
        h = h * 31 + (unsigned char)*key++;
    }
    return h % TABLE_SIZE;

}
/*
 * Hash Table Initialization
 * ------------------------------------
 * Allocates a zero-initialized HashMap
 */
HashTable* hash_table_init(void){

    return(HashTable*)calloc(1, sizeof(HashTable));

}
/*
 * Insert/Update Record in HashMap
 * -------------------------------------------------------
 * If phone already exists --> update score + report count
 * Otherwise --> create a new ScamRecord and insert
 */
void hash_table_insert(HashTable *table, const char *phone, float score, int reports){

    if(!table || !phone) return;

    unsigned int index = hash_function(phone);
    ScamRecord *current = table->buckets[index];

    // Update existing record
    while(current){
        if(strcmp(current->phone, phone) == 0){
            current->suspicious_score = score;
            current->report_count = reports;
            return;
        }
        current = current->next;
    }

    // Insert new record
    ScamRecord *new_rec = (ScamRecord*)malloc(sizeof(ScamRecord));
    if(!new_rec) return;

    strncpy(new_rec->phone, phone, MAX_PHONE_LENGTH);
    new_rec->phone[MAX_PHONE_LENGTH - 1] = '\0';
    new_rec->suspicious_score = score;
    new_rec->report_count = reports;
    new_rec->next = table->buckets[index];
    table->buckets[index] = new_rec;

}
/*
 * Lookup Record by Phone Number
 * ------------------------------------------------
 * Returns pointer to ScamRecord if found else NULL
 */
ScamRecord* hash_table_lookup(HashTable *table, const char *phone){

    if(!table || !phone) return NULL;

    unsigned int index = hash_function(phone);
    ScamRecord *current = table->buckets[index];

    while(current){
        if(strcmp(current->phone, phone) == 0){
            return current;
        }
        current = current->next;
    }
    return NULL;

}
/*
 * Delete Record by Phone Number
 * -------------------------------------------------
 * Removes a scam record from the hash table.
 * Returns 1 if successfully deleted, 0 if not found.
 */
int hash_table_delete(HashTable *table, const char *phone){

    if(!table || !phone) return 0;

    unsigned int index = hash_function(phone);
    ScamRecord *prev = NULL;
    ScamRecord *curr = table->buckets[index];

    while(curr){
        if(strcmp(curr->phone, phone) == 0){
            if(prev) prev->next = curr->next;
            else      table->buckets[index] = curr->next;
            free(curr);
            return 1;
        }
        prev = curr;
        curr = curr->next;
    }
    return 0;

}
/*
 * Free Hash Table
 * --------------------------------------------------
 * Releases all memory associated with the Hash Table
 */
void hash_table_free(HashTable *table){

    if(!table) return;

    for(int i = 0; i < TABLE_SIZE; ++i){
        ScamRecord *current = table->buckets[i];
        while(current){
            ScamRecord *next = current->next;
            free(current);
            current = next;
        }
    }

    free(table);

}