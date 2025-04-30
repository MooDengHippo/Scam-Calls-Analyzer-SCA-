#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_admin.h"
#include "phone_format.h"
#include "hash_table.h"
#include "graph.h"
#include "logging.h"
#define PENDING_FILE "data/pending_reports.csv"
#define ARCHIVE_FILE "data/reports_archive.csv"

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

static void view_pending_reports(HashTable *table){
    FILE *fp = fopen(PENDING_FILE, "r");
    if(!fp){
        puts("No pending reports found.");
        Logging_Write(LOG_WARN, "Admin tried to view missing pending_reports.csv");
        return;
    }

    puts("\n--- Pending Reports ---");
    char lines[100][128];
    int line_num = 0;
    while(fgets(lines[line_num], sizeof(lines[line_num]), fp)){
        printf(" %d) %s", line_num + 1, lines[line_num]);
        line_num++;
    }
    fclose(fp);
    Logging_Write(LOG_INFO, "Admin viewed pending reports");

    if(line_num == 0){ puts("No entries to process."); return; }

    printf("\nSelect report number to accept (0 to skip): ");
    int choice = 0;
    scanf("%d%*c", &choice);
    if(choice <= 0 || choice > line_num) return;

    // Parse accepted line to extract phone number
    char accepted_line[128];
    strcpy(accepted_line, lines[choice - 1]);
    char *phone_token = strtok(accepted_line, ",\n");
    if(phone_token){
        char norm[MAX_PHONE_LENGTH];
        if(Normalize_Phone(phone_token, norm, sizeof(norm)) == 0){
            ScamRecord *rec = hash_table_lookup(table, norm);
            if(rec){
                rec->report_count++;
                rec->suspicious_score = calculate_score(norm, rec->report_count);
                printf("[Updated] %s now has %d reports and score %.2f\n", norm, rec->report_count, rec->suspicious_score);
                Logging_Write(LOG_INFO, "Accepted report applied to existing: %s (%.2f, %d)", norm, rec->suspicious_score, rec->report_count);
            }else{
                float score = calculate_score(norm, 1);
                hash_table_insert(table, norm, score, 1);
                printf("[Added] New record %s with 1 report and score %.2f\n", norm, score);
                Logging_Write(LOG_INFO, "Accepted report added new record: %s (%.2f, 1)", norm, score);
            }
        }
    }

    // Rewrite file excluding accepted line
    FILE *out = fopen("data/tmp.csv", "w");
    FILE *arc = fopen(ARCHIVE_FILE, "a");
    if(!out || !arc){
        puts("File error during processing!");
        if(out) fclose(out);
        if(arc) fclose(arc);
        return;
    }

    for(int i = 0; i < line_num; ++i){
        if(i == choice - 1){ fputs(lines[i], arc); } 
        else { fputs(lines[i], out); }
    }
    fclose(out);
    fclose(arc);
    remove(PENDING_FILE);
    rename("data/tmp.csv", PENDING_FILE);
    puts("Report accepted, archived, and applied to database.");
}

static void analyze_number(HashTable *table, GraphNode *nodes[]){
    char raw[64];
    printf("Enter phone to analyze (or 'q' to cancel): ");
    if(!fgets(raw, sizeof raw, stdin)) return;
    if(strncmp(raw, "q", 1) == 0 || strncmp(raw, "Q", 1) == 0) return;

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

        printf("Do you want to increase report count by 1 and auto-recalculate score? (y/n): ");
        char ans[8];
        if(fgets(ans, sizeof ans, stdin) && (ans[0]=='y' || ans[0]=='Y')){
            rec->report_count++;
            rec->suspicious_score = calculate_score(norm, rec->report_count);
            printf("Updated report count to %d and new score to %.2f\n", rec->report_count, rec->suspicious_score);
            Logging_Write(LOG_INFO, "Admin incremented report and updated score: %s (%.2f, %d)", norm, rec->suspicious_score, rec->report_count);
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

        float est = calculate_score(norm, 1);
        hash_table_insert(table, norm, est, 1);
        printf("Added new record with 1 report and score: %.2f\n", est);
        Logging_Write(LOG_INFO, "Admin added new record via analyze: %s (%.2f, 1)", norm, est);
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
            char phone[64];
            printf("Phone (or 'q' to cancel): ");
            if(!fgets(phone, sizeof phone, stdin)) break;
            if(phone[0] == 'q' || phone[0] == 'Q') continue;
            phone[strcspn(phone, "\n")] = 0;

            char norm[MAX_PHONE_LENGTH];
            if(Normalize_Phone(phone, norm, sizeof(norm)) < 0){
                puts("Invalid phone format!");
                continue;
            }

            ScamRecord *old = hash_table_lookup(table, norm);
            if(old){
                old->report_count++;
                old->suspicious_score = calculate_score(norm, old->report_count);
                printf("Updated existing record: %s (Score: %.2f, Reports: %d)\n", norm, old->suspicious_score, old->report_count);
                Logging_Write(LOG_INFO, "Admin incremented report on existing: %s (%.2f, %d)", norm, old->suspicious_score, old->report_count);
            } else {
                float score = calculate_score(norm, 1);
                hash_table_insert(table, norm, score, 1);
                printf("Added new record: %s (Score: %.2f, Reports: 1)\n", norm, score);
                Logging_Write(LOG_INFO, "Admin added new record: %s (%.2f, 1)", norm, score);
            }

        }else if(choice == 2){
            char phoneA[64], phoneB[64];
            printf("Phone A (or 'q' to cancel): ");
            if(!fgets(phoneA, sizeof phoneA, stdin) || phoneA[0] == 'q' || phoneA[0] == 'Q') continue;
            printf("Phone B (or 'q' to cancel): ");
            if(!fgets(phoneB, sizeof phoneB, stdin) || phoneB[0] == 'q' || phoneB[0] == 'Q') continue;

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
            view_pending_reports(table);
        }else if(choice == 5){
            analyze_number(table, nodes);
        }else{
            break;
        }
    }
}