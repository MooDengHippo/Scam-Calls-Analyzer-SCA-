#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_manage.h"
#include "phone_format.h"
#include "hash_table.h"
#include "graph.h"

/*
 * Trim
 * ------------------------------------------------------
 * Remove leading and trailing whitespace, tabs, newlines
 * Parameters:
 *   str -> Input string to trim
 * Returns:
 *   A pointer to the trimmed string
 */
static char *trim(char *s){

    while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r') s++;
    char *e = s + strlen(s) - 1;
    while (e > s && (*e == ' ' || *e == '\t' || *e == '\n' || *e == '\r')) *e-- = '\0';
    return s;

}
/*
 * Read CSV and populate Hash Table and Graph structure
 * - Format:
 *   R, <phone>, <score 0–1>, <reports> -> Record of suspicious number
 *   E, <phoneA>, <phoneB>              -> Relationship between numbers
 * - Automatically normalize phone numbers
 * - Boosts risk if number is not Thai (+66)
 *   If not SEA --> even higher risk boost
 */
int csv_read_data(const char *record_file, const char *edge_file, HashTable *table, GraphNode *nodes[]){
    
    // Step 1: Read graph edges
    FILE *fe = fopen(edge_file, "r");
    if(!fe){
        perror("Edge CSV open!");
        return -1;
    }

    char line[256];
    while(fgets(line, sizeof(line), fe)){
        char *tok = strtok(line, ",");
        if (!tok) continue;
        tok = trim(tok);

        if(*tok == '#' || strcmp(tok, "E") != 0) continue;

        char *a = strtok(NULL, ",");
        char *b = strtok(NULL, ",");
        if(!a || !b) continue;

        a = trim(a);
        b = trim(b);

        char na[MAX_PHONE_LENGTH], nb[MAX_PHONE_LENGTH];
        if(Normalize_Phone(a, na, sizeof(na)) < 0 || Normalize_Phone(b, nb, sizeof(nb)) < 0) continue;

        graph_add_edge(nodes, na, nb);
    }
    fclose(fe);

    // Step 2: Read suspicious phone records
    FILE *fr = fopen(record_file, "r");
    if(!fr){
        perror("Record CSV open!");
        return -1;
    }

    int cnt = 0;
    while(fgets(line, sizeof(line), fr)){
        char *tok = strtok(line, ",");
        if(!tok) continue;
        tok = trim(tok);

        if(*tok == '#' || strcmp(tok, "R") != 0) continue;

        char *p = strtok(NULL, ",");
        char *score = strtok(NULL, ",");
        char *rep = strtok(NULL, ",");
        if(!p || !score) continue;

        p = trim(p);
        score = trim(score);

        char norm[MAX_PHONE_LENGTH];
        if(Normalize_Phone(p, norm, sizeof(norm)) < 0) continue;

        int rc = rep ? atoi(trim(rep)) : 1;
        GraphNode *node = graph_get_node(nodes, norm);
        int nc = node ? node->neighbor_count : 0;
        float sc = calculate_score(norm, rc, nc);

        hash_table_insert(table, norm, sc, rc);
        cnt++;
    }
    fclose(fr);
    return cnt;

}
/*
 * Write phone records from hash table to CSV
 * Format: R, <phone>, <score>, <report_count>
 */
int csv_write_data(const char *fname, HashTable *table){

    FILE *fp = fopen(fname, "w");
    if(!fp) return -1;

    for(int i = 0; i < TABLE_SIZE; ++i){
        ScamRecord *rec = table->buckets[i];
        while (rec) {
            fprintf(fp, "R,%s,%.2f,%d\n", rec->phone, rec->suspicious_score, rec->report_count);
            rec = rec->next;
        }
    }
    fclose(fp);
    return 0;

}
/*
 * Write graph relationships to CSV
 * Format: E, <phoneA>, <phoneB>
 */
int csv_write_edges(const char *fname, GraphNode *nodes[]){

    FILE *fp = fopen(fname, "w");
    if(!fp) return -1;

    for(int i = 0; i < MAX_NODES; ++i){
        if(nodes[i]){
            GraphNode *n = nodes[i];
            for(int j = 0; j < n->neighbor_count; ++j){
                if(strcmp(n->phone, n->neighbors[j]->phone) < 0)
                    fprintf(fp, "E,%s,%s\n", n->phone, n->neighbors[j]->phone);
            }
        }
    }
    fclose(fp);
    return 0;

}
// Remove a line from pending_reports.csv by index
void remove_pending_index(int index){

    FILE *fp = fopen("data/pending_reports.csv", "r");
    if(!fp) return;

    FILE *temp = fopen("data/pending_temp.csv", "w");
    if(!temp){
        fclose(fp);
        return;
    }

    char line[256];
    int current = 0;
    while(fgets(line, sizeof line, fp)){
        if(current != index){
            fputs(line, temp);
        }
        current++;
    }

    fclose(fp);
    fclose(temp);
    remove("data/pending_reports.csv");
    rename("data/pending_temp.csv", "data/pending_reports.csv");

}