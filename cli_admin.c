#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_admin.h"
#include "phone_format.h"
#include "hash_table.h"
#include "graph.h"
#include "logging.h"

// Analyze new phone number
static float estimate_score(const char *phone, GraphNode *nodes[], HashTable *table){

    float base_score = 0.3f;

    if(!Is_SEA_Country(phone)){
        base_score = 0.7f;
    }else if(strncmp(phone, "+66", 3) != 0){
        base_score = 0.5f;
    }

    GraphNode *node = graph_get_node(nodes, phone);
    if(node && node->neighbor_count > 0){
        float total = 0;
        int count = 0;
        for(int i = 0; i < node->neighbor_count; i++){
            ScamRecord *rec = hash_table_lookup(table, node->neighbors[i]->phone);
            if(rec){
                total += rec->suspicious_score;
                count++;
            }
        }
        if(count > 0){
            float avg = total / count;
            if(avg > base_score)
                base_score = avg;
        }
    }

    return base_score > 1.0f ? 1.0f : base_score;

}

// View user submitted reports
static void view_pending_reports(){

    FILE *fp = fopen("data/pending_reports.csv", "r");
    if(!fp){
        puts("No pending reports found.");
        Logging_Write(LOG_WARN, "Admin tried to view missing pending_reports.csv");
        return;
    }

    puts("\n--- Pending Reports ---");
    char line[128];
    while(fgets(line, sizeof(line), fp)){
        printf(" - %s", line);
    }
    fclose(fp);
    Logging_Write(LOG_INFO, "Admin viewed pending reports");

}

// Analyze number in hash table and graph
static void analyze_number(HashTable *table, GraphNode *nodes[]){

    char raw[64];
    printf("Enter phone to analyze: ");
    if(!fgets(raw, sizeof raw, stdin)) return;

    char norm[MAX_PHONE_LENGTH];
    if(Normalize_Phone(raw, norm, sizeof(norm)) < 0){
        puts("Invalid phone format!");
        Logging_Write(LOG_WARN, "Admin analyze failed, invalid phone: %s", raw);
        return;
    }

    ScamRecord *rec = hash_table_lookup(table, norm);
    if(rec){
        printf("\nPhone: %s\nRisk Score: %.2f\nReports: %d\n",
            rec->phone, rec->suspicious_score, rec->report_count);
        Logging_Write(LOG_INFO, "Admin analyzed (found): %s", norm);
    }else{
        GraphNode *node = graph_get_node(nodes, norm);
        if(node && node->neighbor_count > 0) {
            printf("\nNo record found, but connected to %d neighbors:\n", node->neighbor_count);
            for(int i = 0; i < node->neighbor_count; i++){
                printf(" - %s\n", node->neighbors[i]->phone);
            }
        }else{
            puts("\nNo record or relationship found.");
        }

        float est = estimate_score(norm, nodes, table);
        printf("Estimated suspicious score: %.2f\n", est);
        printf("Would you like to add this number to the database? (y/n): ");

        char ans[8];
        if(fgets(ans, sizeof ans, stdin) && (ans[0] == 'y' || ans[0] == 'Y')){
            hash_table_insert(table, norm, est, 1);
            printf("Inserted %s with score %.2f and 1 report.\n", norm, est);
            Logging_Write(LOG_INFO, "Admin inserted new record via analyze: %s (%.2f)", norm, est);
        }else{
            puts("Skipped adding to database.");
        }

        Logging_Write(LOG_INFO, "Admin analyzed (not found): %s", norm);
    }

}

// Main CLI for Admin
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
            if(Normalize_Phone(phone, norm, sizeof(norm)) < 0) {
                puts("Invalid phone!");
                Logging_Write(LOG_WARN, "Admin entered invalid phone: %s", phone);
                continue;
            }

            float sc = atof(risk_str);
            if(sc < 0.0f || sc > 1.0f){
                puts("Invalid risk score! Must be between 0 and 1.");
                Logging_Write(LOG_WARN, "Admin entered invalid score: %s", norm);
                continue;
            }

            int rc = atoi(rep_str);
            hash_table_insert(table, norm, sc, rc);
            printf("Added %s\n", norm);
            Logging_Write(LOG_INFO, "Admin added record: %s (%.2f, %d)", norm, sc, rc);

        }else if(choice == 2){
            char p1[64], p2[64];
            printf("Phone 1: ");
            if(!fgets(p1, sizeof(p1), stdin)) break;
            printf("Phone 2: ");
            if(!fgets(p2, sizeof(p2), stdin)) break;

            char n1[MAX_PHONE_LENGTH], n2[MAX_PHONE_LENGTH];
            if(Normalize_Phone(p1, n1, sizeof(n1)) < 0 || Normalize_Phone(p2, n2, sizeof(n2)) < 0) {
                puts("One or both phones invalid!");
                Logging_Write(LOG_WARN, "Admin tried linking invalid: %s - %s", p1, p2);
                continue;
            }

            graph_add_edge(nodes, n1, n2);
            printf("Linked %s ↔ %s\n", n1, n2);
            Logging_Write(LOG_INFO, "Admin linked: %s ↔ %s", n1, n2);

        }else if(choice == 3) {
            view_pending_reports();
        }else if(choice == 4) {
            analyze_number(table, nodes);
        }else{
            break;
        }
    }
}