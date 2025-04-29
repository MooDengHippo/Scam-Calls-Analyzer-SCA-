#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_admin.h"
#include "phone_format.h"
#include "hash_table.h"
#include "graph.h"
#include "logging.h"

// Show all pending reports from user
static void view_pending_reports(void){

    FILE *fp = fopen("data/pending_reports.csv", "r");
    if(!fp){
        puts("No pending reports found.");
        Logging_Write(LOG_WARN, "Admin tried to view missing pending_reports.csv");
        return;
    }
    puts("\n--- Pending Reports ---");
    char line[64];
    while(fgets(line, sizeof(line), fp)){
        printf(" - %s", line);
    }
    fclose(fp);
    Logging_Write(LOG_INFO, "Admin viewed pending reports");

}

// Analyze a specific phone number
static void analyze_number(HashTable *table, GraphNode *nodes[]){

    char raw[64];
    printf("Enter phone to analyze: ");
    if(!fgets(raw, sizeof raw, stdin)) return;

    char norm[MAX_PHONE_LENGTH];
    if(Normalize_Phone(raw, norm, sizeof(norm)) < 0) {
        puts("Invalid phone format!");
        Logging_Write(LOG_WARN, "Admin analyze failed, invalid phone: %s", raw);
        return;
    }

    ScamRecord *rec = hash_table_lookup(table, norm);
    if(rec){
        printf("\nPhone: %s\nRisk Score: %.2f\nReports: %d\n",
               rec->phone, rec->suspicious_score, rec->report_count);
        Logging_Write(LOG_INFO, "Admin analyzed number (record found): %s", norm);
    }else{
        GraphNode *node = graph_get_node(nodes, norm);

        if(node && node->neighbor_count > 0){
            printf("\nNo record found, but connected to %d neighbors:\n", node->neighbor_count);
            for(int i = 0; i < node->neighbor_count; i++){
                printf(" - %s\n", node->neighbors[i]->phone);
            }
            Logging_Write(LOG_INFO, "Admin analyzed number (graph found): %s", norm);
        }else{
            puts("\nNo record or relationship found.");
            Logging_Write(LOG_INFO, "Admin analyzed number (no data): %s", norm);
        }

    }

}

// Admin Mode Main Handler
void admin_mode(HashTable *table, GraphNode *nodes[]){

    while(1){
        puts("\n--- Admin Menu ---");
        puts(" 1) Add suspicious phone record");
        puts(" 2) Add relationship edge");
        puts(" 3) View pending reports");
        puts(" 4) Analyze number");
        puts(" 5) Back to main menu");
        printf("Select: ");

        int choice = 0;
        if(scanf("%d%*c", &choice) != 1){
            choice = 5;
        }

        if(choice == 1){
            char phone[64], risk_str[16], rep_str[16];
            printf("Phone: ");
            if(!fgets(phone, sizeof(phone), stdin)) break;
            printf("Risk score (0-1): ");
            if(!fgets(risk_str, sizeof(risk_str), stdin)) break;
            printf("Report count: ");
            if(!fgets(rep_str, sizeof(rep_str), stdin)) break;

            char norm[MAX_PHONE_LENGTH];
            if(Normalize_Phone(phone, norm, sizeof(norm)) < 0){
                puts("Invalid phone!");
                Logging_Write(LOG_WARN, "Admin entered invalid phone: %s", phone);
                continue;
            }

            float sc = atof(risk_str);
            if(sc < 0.0f || sc > 1.0f){
                puts("Invalid risk score! Must be between 0 and 1.");
                Logging_Write(LOG_WARN, "Admin entered invalid score for: %s", norm);
                continue;
            }

            int rc = atoi(rep_str);
            hash_table_insert(table, norm, sc, rc);
            printf("Added %s\n", norm);
            Logging_Write(LOG_INFO, "Admin added record: %s (score: %.2f, reports: %d)", norm, sc, rc);

        }else if(choice == 2){
            char p1[64], p2[64];
            printf("Phone 1: ");
            if(!fgets(p1, sizeof(p1), stdin)) break;
            printf("Phone 2: ");
            if(!fgets(p2, sizeof(p2), stdin)) break;

            char n1[MAX_PHONE_LENGTH], n2[MAX_PHONE_LENGTH];
            if(Normalize_Phone(p1, n1, sizeof(n1)) < 0 ||
                Normalize_Phone(p2, n2, sizeof(n2)) < 0){
                puts("One or both phones invalid!");
                Logging_Write(LOG_WARN, "Admin tried linking invalid phones: %s - %s", p1, p2);
                continue;
            }

            graph_add_edge(nodes, n1, n2);
            printf("Linked %s \u2194 %s\n", n1, n2);
            Logging_Write(LOG_INFO, "Admin linked numbers: %s \u2194 %s", n1, n2);

        }else if(choice == 3){
            view_pending_reports();
        }else if(choice == 4){
            analyze_number(table, nodes);
        }else{
            break;
        }
    }

}