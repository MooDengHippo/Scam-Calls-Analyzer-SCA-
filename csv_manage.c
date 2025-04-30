#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_manage.h"
#include "phone_format.h"
#include "hash_table.h"
// Calculate risk score based on phone pattern and report count
static float calculate_score(const char *phone, int report_count){

    if (!Is_SEA_Country(phone)) return 1.0f;
    if (strncmp(phone, "+66", 3) == 0) {
        if (strncmp(phone, "+662", 4) == 0) return fminf(1.0f, 0.5f + 0.05f * report_count);
        return fminf(1.0f, 0.1f + 0.05f * report_count);
    }
    if (strncmp(phone, "+855", 4) == 0 || strncmp(phone, "+95", 3) == 0 || strncmp(phone, "+856", 4) == 0)
        return fminf(1.0f, 0.7f + 0.05f * report_count);
    return fminf(1.0f, 0.8f + 0.05f * report_count);

}
/*
 * Trim
 * -------------------------
 * Remove leading and trailing whitespace (spaces, tabs, newlines).
 * Parameters:
 *   str -> Input string to trim
 * Returns:
 *   A pointer to the trimmed string
 */
static char *trim(char *s){

    while(*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    char *e = s + strlen(s) - 1;
    while(e > s && (*e == ' ' || *e == '\t' || *e == '\n' || *e == '\r')) *e-- = '\0';
    return s;

}
/*
 * Read CSV and populate Hash Table and Graph structure
 * - Format:
 *   R, <phone>, <score 0‑1>, <reports> // Record of suspicious number
 *   E, <phoneA>, <phoneB>              // Relationship between numbers
 * - Automatically normalize phone numbers
 * - Boosts risk if number is not Thai (+66)
 *   If not SEA --> even higher risk boost
 */
int csv_read_data(const char *fname, HashTable *table, GraphNode *nodes[]){

    FILE *fp = fopen(fname, "r");
    if(!fp){
        perror("CSV open!");
        return -1;
    }

    char line[256];
    int cnt = 0;

    while(fgets(line, sizeof(line), fp)){
        char *tok = strtok(line, ",");
        if(!tok) continue;
        tok = trim(tok);

        if(*tok == '#') continue; // Ignore comment lines

        if(strcmp(tok, "R") == 0){
            // Handle record row: R,<phone>,<score>,<report_count>
            char *p = strtok(NULL, ",");
            char *score = strtok(NULL, ",");
            char *rep = strtok(NULL, ",");
            if(!p || !score) continue;

            p = trim(p);
            score = trim(score);

            char norm[MAX_PHONE_LENGTH];
            if(Normalize_Phone(p, norm, sizeof(norm)) < 0) continue;

            int rc = rep ? atoi(trim(rep)) : 1;
            float sc = calculate_score(norm, rc);

            hash_table_insert(table, norm, sc, rc);
            cnt++;

        }else if(strcmp(tok, "E") == 0){
            // Handle edge row: E,<phoneA>,<phoneB>
            char *a = strtok(NULL, ",");
            char *b = strtok(NULL, ",");
            if(!a || !b) continue;

            a = trim(a);
            b = trim(b);

            char na[MAX_PHONE_LENGTH], nb[MAX_PHONE_LENGTH];
            if(Normalize_Phone(a, na, sizeof(na)) < 0 || Normalize_Phone(b, nb, sizeof(nb)) < 0) continue;

            graph_add_edge(nodes, na, nb);
            cnt++;
        }
    }
    fclose(fp);
    return cnt;

}
/*
 * Write current map data back to CSV
 * - Format: R,<phone>,<score>,<report_count>
 * - Used on program exit to persist data
 */
int csv_write_data(const char *fname, HashTable *table){

    FILE *fp = fopen(fname, "w");
    if(!fp){
        perror("CSV write");
        return -1;
    }

    for(int i = 0; i < TABLE_SIZE; ++i){
        ScamRecord *rec = table->buckets[i];
        while(rec){
            fprintf(fp, "R,%s,%.2f,%d\n", rec->phone, rec->suspicious_score, rec->report_count);
            rec = rec->next;
        }
    }
    fclose(fp);
    return 0;

}