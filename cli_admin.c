#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_admin.h"
#include "phone_format.h"
#include "hash_table.h"
#include "graph.h"
#include "logging.h"

// Custom score calculation based on phone type and reports
static float calculate_score(const char *phone, int report_count){

    if(!Is_SEA_Country(phone)) return 1.0f;
    if(strncmp(phone, "+66", 3) == 0){
        if (strncmp(phone, "+662", 4) == 0) return fminf(1.0f, 0.5f + 0.05f * report_count);
        return fminf(1.0f, 0.1f + 0.05f * report_count);
    }
    if(strncmp(phone, "+855", 4) == 0 || strncmp(phone, "+95", 3) == 0 || strncmp(phone, "+856", 4) == 0)
        return fminf(1.0f, 0.7f + 0.05f * report_count);
    return fminf(1.0f, 0.8f + 0.05f * report_count);

}

static float estimate_score(const char *phone, GraphNode *nodes[], HashTable *table){

    float base = calculate_score(phone, 0);
    GraphNode *node = graph_get_node(nodes, phone);
    if(node){
        float tot = 0; int cnt = 0;
        for(int i = 0; i < node->neighbor_count; ++i){
            ScamRecord *r = hash_table_lookup(table, node->neighbors[i]->phone);
            if(r){ tot += r->suspicious_score; cnt++; }
        }
        if(cnt) base = fmaxf(base, tot / cnt);
    }
    return base > 1.0f ? 1.0f : base;

}

static void view_pending_reports(){

    FILE *fp = fopen("data/pending_reports.csv", "r");
    if(!fp){
        puts("No pending reports found.");
        Logging_Write(LOG_WARN, "Admin tried to view missing pending_reports.csv");
        return;
    }
    puts("\n--- Pending Reports ---");
    char line[128];
    while(fgets(line, sizeof line, fp))
        printf(" - %s", line);
    fclose(fp);
    Logging_Write(LOG_INFO, "Admin viewed pending reports");

}

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

        printf("Do you want to update this record? (y/n): ");
        char ans[8];
        if(fgets(ans, sizeof ans, stdin) && (ans[0]=='y' || ans[0]=='Y')){
            char risk_str[16], rep_str[16];
            printf("New Risk score (0-1): ");
            if(!fgets(risk_str, sizeof risk_str, stdin)) return;
            printf("New Report count: ");
            if(!fgets(rep_str, sizeof rep_str, stdin)) return;

            float sc = atof(risk_str);
            int rc = atoi(rep_str);
            hash_table_insert(table, norm, sc, rc);
            printf("Record updated.\n");
            Logging_Write(LOG_INFO, "Admin updated record: %s (%.2f, %d)", norm, sc, rc);
        }
    }else{
        GraphNode *node = graph_get_node(nodes, norm);
        if(node && node->neighbor_count){
            printf("\nNo record found, but connected to %d neighbors:\n",
                   node->neighbor_count);
            for(int i=0; i<node->neighbor_count; ++i)
                printf(" - %s\n", node->neighbors[i]->phone);
        }else{
            puts("\nNo record or relationship found.");
        }

        float est = estimate_score(norm, nodes, table);
        printf("Estimated suspicious score: %.2f\n", est);
        Logging_Write(LOG_INFO, "Admin analyzed (not found): %s", norm);
    }

}

void admin_mode(HashTable *table, GraphNode *nodes[]){

    while(1){
        puts("\n--- Admin Menu ---");
        puts(" 1) Add/update suspicious phone record");
        puts(" 2) Add relationship edge");
        puts(" 3) Delete suspicious phone record");
        puts(" 4) View pending reports");
        puts(" 5) Analyze number");
        puts(" 6) Back to main menu");
        printf("Select: ");

        int choice = 0;
        if(scanf("%d%*c", &choice) != 1) choice = 6;

        if(choice == 1){
            char phone[64], risk_str[16], rep_str[16];
            printf("Phone: ");
            if(!fgets(phone, sizeof phone, stdin)) break;
            phone[strcspn(phone, "\n")] = 0;

            char norm[MAX_PHONE_LENGTH];
            if(Normalize_Phone(phone, norm, sizeof(norm)) < 0){
                puts("Invalid phone format!");
                continue;
            }

            ScamRecord *old = hash_table_lookup(table, norm);
            if(old){
                printf("Record exists (score %.2f, reports %d). Update? (y/n): ",
                       old->suspicious_score, old->report_count);
                char ua[8];
                if(fgets(ua, sizeof ua, stdin)
                   && (ua[0]!='y' && ua[0]!='Y')){
                    puts("Skipped update.");
                    continue;
                }
            }

            printf("Risk score (0-1): ");
            if(!fgets(risk_str, sizeof risk_str, stdin)) break;
            printf("Report count: ");
            if(!fgets(rep_str, sizeof rep_str, stdin)) break;

            float sc = atof(risk_str);
            int   rc = atoi(rep_str);
            hash_table_insert(table, norm, sc, rc);
            printf("%s record %s.\n", old ? "Updated" : "Added", norm);
            Logging_Write(LOG_INFO, "Admin %s record: %s (%.2f, %d)",
                          old ? "updated" : "added", norm, sc, rc);

        }else if(choice == 2){
            char phoneA[64], phoneB[64];
            printf("Phone A: ");
            if(!fgets(phoneA, sizeof phoneA, stdin)) continue;
            printf("Phone B: ");
            if(!fgets(phoneB, sizeof phoneB, stdin)) continue;

            char normA[MAX_PHONE_LENGTH], normB[MAX_PHONE_LENGTH];
            if(Normalize_Phone(phoneA, normA, sizeof(normA)) < 0 ||
               Normalize_Phone(phoneB, normB, sizeof(normB)) < 0){
                puts("Invalid phone format!");
                continue;
            }

            graph_add_edge(nodes, normA, normB);
            printf("Linked %s <--> %s\n", normA, normB);
            Logging_Write(LOG_INFO, "Admin linked %s <--> %s", normA, normB);

        }else if(choice == 3){
            printf("Phone to delete (q to cancel): ");
            char dp[64];
            if(!fgets(dp, sizeof dp, stdin) || dp[0]=='q') continue;
            dp[strcspn(dp, "\n")] = 0;

            char dn[MAX_PHONE_LENGTH];
            if(Normalize_Phone(dp, dn, sizeof(dn)) < 0){
                puts("Invalid phone format!");
                continue;
            }
            if(hash_table_delete(table, dn)){
                printf("Deleted %s\n", dn);
                Logging_Write(LOG_INFO, "Admin deleted record: %s", dn);
            } else {
                puts("Record not found!");
            }

        }else if(choice == 4){
            view_pending_reports();
        }else if(choice == 5){
            analyze_number(table, nodes);
        }else{
            break;
        }
    }
    
}