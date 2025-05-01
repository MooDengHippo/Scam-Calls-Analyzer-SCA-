#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cli_user.h"
#include "phone_format.h"
#include "hash_table.h"
#include "graph.h"
#include "logging.h"

// Convert suspicious score to textual risk level
static const char* get_risk_level_description(float score){

    if(score >= 0.81f) return "SEVERE";
    else if(score >= 0.61f) return "HIGH";
    else if(score >= 0.41f) return "MEDIUM";
    else if(score >= 0.21f) return "LOW";
    else return "VERY LOW";

}
// Display suspicious score as a bar
static void display_suspicious_score(float score){

    int bar = (int)(score * 20);
    printf("\nSuspicious Score: %6.2f %%\n[", score * 100);
    for(int i = 0; i < 20; ++i)
        putchar(i < bar ? '#' : '-');
    puts("]");
    printf("Risk Level: %s\n\n", get_risk_level_description(score));

}
// Reset visited flags
static void reset_graph_visits(GraphNode *nodes[]){

    for(int i = 0; i < MAX_NODES; ++i)
        if(nodes[i]) nodes[i]->visited = 0;

}
// Recursive graph display with ASCII
static void display_scam_graph(GraphNode *node, int level){

    if(!node || node->visited) return;
    node->visited = 1;
    for(int i = 0; i < level; ++i) printf("  ");
    printf("-- %s\n", node->phone);
    for(int i = 0; i < node->neighbor_count; ++i)
        display_scam_graph(node->neighbors[i], level + 1);

}
// Append a report to pending_reports.csv only
static void report_number(const char *phone){

    FILE *fp = fopen("data/pending_reports.csv", "a");
    if(!fp){ perror("Could not open pending_reports.csv"); return; }

    time_t raw = time(NULL);
    struct tm *t = localtime(&raw);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
    fprintf(fp, "%s,%s\n", phone, timestamp);
    fclose(fp);
    Logging_Write(LOG_INFO, "User reported number: %s", phone);

}
// User CLI loop
void user_mode(HashTable *table, GraphNode *nodes[]){

    while(1){
        char raw[64];
        printf("\nEnter phone (q to return): ");
        if(!fgets(raw, sizeof raw, stdin)) break;
        if(strncmp(raw, "q", 1) == 0 || strncmp(raw, "Q", 1) == 0){
            puts("");
            Logging_Write(LOG_INFO, "User exited User Mode");
            break;
        }

        char norm[MAX_PHONE_LENGTH];
        if(Normalize_Phone(raw, norm, sizeof(norm)) < 0){
            puts("Invalid phone format!\n");
            Logging_Write(LOG_WARN, "User entered invalid phone: %s", raw);
            continue;
        }
        Logging_Write(LOG_INFO, "User searched for: %s", norm);

        ScamRecord *rec = hash_table_lookup(table, norm);
        if(rec){
            printf("\nNumber found (reported %d times)\n", rec->report_count);
            display_suspicious_score(rec->suspicious_score);

            printf("Send this number to admin for further inspection? (y/n): ");
            char ans2[8];
            if(fgets(ans2, sizeof ans2, stdin) && (ans2[0]=='y' || ans2[0]=='Y')){
                report_number(norm);
                puts("Number sent to admin. Thank you!\n");
            }

            Logging_Write(LOG_INFO, "Phone found: %s (score: %.2f, reports: %d)",
                          norm, rec->suspicious_score, rec->report_count);

        }else{
            if(!Is_SEA_Country(norm)){
                puts("\nForeign (Non-SEA) number - HIGH RISK!\n");
                Logging_Write(LOG_WARN, "Foreign number - Not SEA: %s", norm);
            }else{
                puts("\nNot found - exploring relationship graph:\n");
                Logging_Write(LOG_INFO, "SEA number not found in DB: %s", norm);
                reset_graph_visits(nodes);
                GraphNode *start = graph_get_node(nodes, norm);
                if(start && start->neighbor_count > 0){
                    display_scam_graph(start, 0);
                } else {
                    puts("No relationships found!\n");
                }
            }

            printf("Would you like to report this number? (y/n): ");
            char ans[8];
            if(fgets(ans, sizeof ans, stdin) && (ans[0] == 'y' || ans[0] == 'Y')){
                report_number(norm);
                puts("Number reported. Thank you!\n");
                Logging_Write(LOG_INFO, "User reported unknown number: %s", norm);
            }
        }
    }

}