#include "hash_map.h"
#include <stdlib.h>
#include <string.h>
/*
 * Hash Function
 * ----------------------------------------
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
 * HashMap Initialization
 * ----------------------------------------
 * Allocates a zero-initialized HashMap
 */
HashMap* hash_map_init(void){

    return(HashMap*)calloc(1, sizeof(HashMap));

}
/*
 * Insert/Update Record in HashMap
 * ----------------------------------------
 * If phone already exists --> update score + report count
 * Otherwise --> create a new ScamRecord and insert
 */
void hash_map_insert(HashMap *map, const char *phone, float score, int reports){

    if(!map || !phone) return;

    unsigned int index = hash_function(phone);
    ScamRecord *current = map->buckets[index];

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
    new_rec->next = map->buckets[index];
    map->buckets[index] = new_rec;

}
/*
 * Lookup Record by Phone Number
 * ----------------------------------------
 * Returns pointer to ScamRecord if found else NULL
 */
ScamRecord* hash_map_lookup(HashMap *map, const char *phone){

    if(!map || !phone) return NULL;

    unsigned int index = hash_function(phone);
    ScamRecord *current = map->buckets[index];

    while(current){
        if(strcmp(current->phone, phone) == 0){
            return current;
        }
        current = current->next;
    }
    return NULL;

}
/*
 * Free HashMap
 * ----------------------------------------
 * Releases all memory associated with the HashMap
 */
void hash_map_free(HashMap *map){

    if(!map) return;

    for(int i = 0; i < TABLE_SIZE; ++i){
        ScamRecord *current = map->buckets[i];
        while(current){
            ScamRecord *next = current->next;
            free(current);
            current = next;
        }
    }
    free(map);

}