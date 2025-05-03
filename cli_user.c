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
    puts("]\n");

}

// Display risk table for a specific record (user version)
static void display_user_risk_table(ScamRecord *rec){

    printf("=============================================\n");
    printf("|  Phone Number   | Reports |  Risk Level   |\n");
    printf("---------------------------------------------\n");
    printf("|  %-15s |   %-6d |  %-11s |\n",
        rec->phone,
        rec->report_count,
        get_risk_level_description(rec->suspicious_score));
    printf("=============================================\n");

}

// Reset visited flags
static void reset_graph_visits(GraphNode *nodes[]){

    for(int i = 0; i < MAX_NODES; ++i)
        if(nodes[i]) nodes[i]->visited = 0;

}

// Enhanced display of scam graph with visual indentation
static void display_graph_ui(GraphNode *node, int level){

    if(!node || node->visited) return;
    node->visited = 1;

    for(int i = 0; i < level; ++i) printf("  ");
    printf("+-- %s\n", node->phone);

    for(int i = 0; i < node->neighbor_count; ++i)
        display_graph_ui(node->neighbors[i], level + 1);

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
            puts("\nInvalid phone format!\n");
            Logging_Write(LOG_WARN, "User entered invalid phone: %s", raw);
            continue;
        }
        Logging_Write(LOG_INFO, "User searched for: %s", norm);

        ScamRecord *rec = hash_table_lookup(table, norm);
        if(rec){
            display_suspicious_score(rec->suspicious_score);
            display_user_risk_table(rec);

            // Show graph relationship even if record exists in DB
            reset_graph_visits(nodes);
            GraphNode *start = graph_get_node(nodes, norm);
            if(start && start->neighbor_count > 0){
                puts("\nConnected numbers in scam relationship:\n");
                display_graph_ui(start, 0);
            }else{
                puts("\nNo known relationships in graph.\n");
            }

            while(1){
                printf("Send this number to admin for further inspection? (y/n): ");
                char ans2[8];
                if(!fgets(ans2, sizeof ans2, stdin)) break;
                size_t len = strlen(ans2);
                if(len == 2 && (ans2[0]=='y' || ans2[0]=='Y')){
                    report_number(norm);
                    puts("\nNumber sent to admin. Thank you!\n");
                    break;
                }else if(len == 2 && (ans2[0]=='n' || ans2[0]=='N')){
                    break;
                }else{
                    puts("\nInvalid input. Please enter y or n.\n");
                }
            }

        }else{
            if(!Is_SEA_Country(norm)){
                puts("\nForeign (Non-SEA) number - DANGER!\n");
                Logging_Write(LOG_WARN, "Foreign number - Not SEA: %s", norm);
            }else{
                puts("\nNot found - exploring relationship graph:\n");
                Logging_Write(LOG_INFO, "SEA number not found in DB: %s", norm);
                reset_graph_visits(nodes);
                GraphNode *start = graph_get_node(nodes, norm);
                if(start && start->neighbor_count > 0){
                    display_graph_ui(start, 0);
                }else{
                    puts("\nNo relationships found!\n");
                }
            }

            while(1){
                printf("Would you like to report this number? (y/n): ");
                char ans[8];
                if(!fgets(ans, sizeof ans, stdin)) break;
                size_t len = strlen(ans);
                if(len == 2 && (ans[0] == 'y' || ans[0] == 'Y')){
                    report_number(norm);
                    puts("\nNumber reported. Thank you!\n");
                    Logging_Write(LOG_INFO, "User reported unknown number: %s", norm);
                    break;
                }else if(len == 2 && (ans[0] == 'n' || ans[0] == 'N')){
                    break;
                }else{
                    puts("\nInvalid input. Please enter y or n.\n");
                }
            }
        }
    }

}