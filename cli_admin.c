#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_admin.h"
#include "phone_format.h"
#include "hash_table.h"
#include "graph.h"
#include "logging.h"
#define PENDING_FILE "data/pending_reports.csv"
#define DB_FILE "data/scam_numbers.csv"

// External CSV writing functions
extern int csv_write_data(const char *fname, HashTable *map);
extern int csv_write_edges(const char *fname, GraphNode **nodes);

static void accept_pending_report(const char *line, HashTable *table, GraphNode *nodes[]) {
    char copy[128];
    strncpy(copy, line, sizeof(copy));
    char *phone_token = strtok(copy, ",\n");
    if (!phone_token) return;

    char norm[MAX_PHONE_LENGTH];
    if (Normalize_Phone(phone_token, norm, sizeof(norm)) < 0) {
        printf("Invalid phone format: %s\n", phone_token);
        Logging_Write(LOG_WARN, "Admin tried to accept invalid phone: %s", phone_token);
        return;
    }

    GraphNode *node = graph_get_node(nodes, norm);
    int neighbors = node ? node->neighbor_count : 0;

    ScamRecord *rec = hash_table_lookup(table, norm);
    if (rec) {
        rec->report_count++;
        rec->suspicious_score = calculate_score(norm, rec->report_count, neighbors);
        printf("[Updated] %s now has %d reports and score %.2f\n", norm, rec->report_count, rec->suspicious_score);
        Logging_Write(LOG_INFO, "Accepted report updated existing: %s (%.2f, %d)", norm, rec->suspicious_score, rec->report_count);
    } else {
        float score = calculate_score(norm, 1, neighbors);
        hash_table_insert(table, norm, score, 1);
        printf("[Added] New record %s with 1 report and score %.2f\n", norm, score);
        Logging_Write(LOG_INFO, "Accepted report added new: %s (%.2f, 1)", norm, score);
    }

    // Save both updated record and relationship graph
    csv_write_data(DB_FILE, table);
    csv_write_edges(DB_FILE, nodes); // <-- Added to ensure graph is saved
}

static void view_pending_reports(HashTable *table, GraphNode *nodes[]) {
    FILE *fp = fopen(PENDING_FILE, "r");
    if (!fp) {
        puts("No pending reports found.");
        Logging_Write(LOG_WARN, "Admin tried to view missing pending_reports.csv");
        return;
    }

    char lines[100][128];
    int line_num = 0;
    puts("\n--- Pending Reports ---");
    while (fgets(lines[line_num], sizeof(lines[line_num]), fp)) {
        printf(" %d) %s", line_num + 1, lines[line_num]);
        line_num++;
    }
    fclose(fp);
    if (line_num == 0) {
        puts("No entries to process.");
        return;
    }
    Logging_Write(LOG_INFO, "Admin viewed pending reports");

    printf("\nSelect report number to accept (q to skip): ");
    char input[16];
    if (!fgets(input, sizeof input, stdin)) return;
    if (input[0] == 'q' || input[0] == 'Q') return;

    int choice = atoi(input);
    if (choice <= 0 || choice > line_num) {
        puts("Invalid selection.");
        return;
    }

    accept_pending_report(lines[choice - 1], table, nodes);

    FILE *out = fopen("data/tmp.csv", "w");
    if (!out) {
        puts("File error during cleanup!");
        return;
    }
    for (int i = 0; i < line_num; ++i) {
        if (i != choice - 1) {
            fputs(lines[i], out);
        }
    }
    fclose(out);
    remove(PENDING_FILE);
    rename("data/tmp.csv", PENDING_FILE);
    puts("Report accepted and applied to database.");

}

static void analyze_number(HashTable *table, GraphNode *nodes[]){
    char raw[64];
    printf("Enter phone to analyze (or 'q' to cancel): ");
    if (!fgets(raw, sizeof raw, stdin)) return;
    if (strncmp(raw, "q", 1) == 0 || strncmp(raw, "Q", 1) == 0) return;

    char norm[MAX_PHONE_LENGTH];
    if (Normalize_Phone(raw, norm, sizeof(norm)) < 0) {
        puts("Invalid phone format!");
        Logging_Write(LOG_WARN, "Admin analyze failed, invalid phone: %s", raw);
        return;
    }

    GraphNode *node = graph_get_node(nodes, norm);
    int neighbors = node ? node->neighbor_count : 0;

    ScamRecord *rec = hash_table_lookup(table, norm);
    if (rec) {
        printf("\nPhone: %s\nRisk Score: %.2f\nReports: %d\n", rec->phone, rec->suspicious_score, rec->report_count);
        Logging_Write(LOG_INFO, "Admin analyzed (found): %s", norm);

        // Show relationships if exist
        if (node && node->neighbor_count > 0) {
            puts("Connected numbers in scam relationship:");
            for (int i = 0; i < node->neighbor_count; ++i)
                printf(" - %s\n", node->neighbors[i]->phone);
        }
    } else {
        if (node && node->neighbor_count > 0) {
            printf("\nNo record found, but connected to %d neighbors:\n", node->neighbor_count);
            for (int i = 0; i < node->neighbor_count; ++i)
                printf(" - %s\n", node->neighbors[i]->phone);
            Logging_Write(LOG_INFO, "Admin analyzed E-only record: %s", norm);
        } else {
            puts("\nNo record or relationship found.");
        }
    }
}

void admin_mode(HashTable *table, GraphNode *nodes[]) {
    while (1) {
        puts("\n--- Admin Menu ---");
        puts(" 1) Add/update suspicious phone record");
        puts(" 2) Add relationship edge");
        puts(" 3) Delete suspicious phone record");
        puts(" 4) View pending reports");
        puts(" 5) Analyze number");
        puts(" 6) Back to main menu");
        printf("Select: ");

        int choice = 0;
        if (scanf("%d%*c", &choice) != 1) choice = 6;

        if (choice == 1) {
            char phone[64];
            printf("Phone (or 'q' to cancel): ");
            if (!fgets(phone, sizeof phone, stdin)) break;
            if (phone[0] == 'q' || phone[0] == 'Q') continue;
            phone[strcspn(phone, "\n")] = 0;

            char norm[MAX_PHONE_LENGTH];
            if (Normalize_Phone(phone, norm, sizeof(norm)) < 0) {
                puts("Invalid phone format!");
                continue;
            }

            GraphNode *node = graph_get_node(nodes, norm);
            int neighbors = node ? node->neighbor_count : 0;

            ScamRecord *old = hash_table_lookup(table, norm);
            if (old) {
                printf("This number already exists with score %.2f and reports %d\n", old->suspicious_score, old->report_count);
                printf("Do you want to increase the report count? (y/n): ");
                char yn[8];
                if (fgets(yn, sizeof yn, stdin) && (yn[0] == 'y' || yn[0] == 'Y')) {
                    old->report_count++;
                    old->suspicious_score = calculate_score(norm, old->report_count, neighbors);
                    printf("Updated record: %s (Score: %.2f, Reports: %d)\n", norm, old->suspicious_score, old->report_count);
                    Logging_Write(LOG_INFO, "Admin incremented report on existing: %s (%.2f, %d)", norm, old->suspicious_score, old->report_count);
                    csv_write_data(DB_FILE, table);
                } else {
                    puts("No changes made.");
                }
            } else {
                float score = calculate_score(norm, 0, neighbors);
                hash_table_insert(table, norm, score, 0);
                printf("Added new record: %s (Score: %.2f, Reports: 0)\n", norm, score);
                Logging_Write(LOG_INFO, "Admin added new record: %s (%.2f, 0)", norm, score);
                csv_write_data(DB_FILE, table);
            }

        } else if (choice == 2) {
            char phoneA[64], phoneB[64];
            printf("Phone A (or 'q' to cancel): ");
            if (!fgets(phoneA, sizeof phoneA, stdin) || phoneA[0] == 'q' || phoneA[0] == 'Q') continue;
            printf("Phone B (or 'q' to cancel): ");
            if (!fgets(phoneB, sizeof phoneB, stdin) || phoneB[0] == 'q' || phoneB[0] == 'Q') continue;

            char normA[MAX_PHONE_LENGTH], normB[MAX_PHONE_LENGTH];
            if (Normalize_Phone(phoneA, normA, sizeof(normA)) < 0 ||
                Normalize_Phone(phoneB, normB, sizeof(normB)) < 0) {
                puts("Invalid phone format!");
                continue;
            }

            // Prevent edge creation between same or invalid short numbers
            if(strlen(normA) < 10 || strlen(normB) < 10){
                puts("Phone number too short to create relationship.");
                continue;
            }

            graph_add_edge(nodes, normA, normB);
            printf("Linked %s <--> %s\n", normA, normB);
            Logging_Write(LOG_INFO, "Admin linked %s <--> %s", normA, normB);
            csv_write_edges(DB_FILE, nodes);

        } else if (choice == 3) {
            printf("Phone to delete (q to cancel): ");
            char dp[64];
            if (!fgets(dp, sizeof dp, stdin) || dp[0] == 'q' || dp[0] == 'Q') continue;
            dp[strcspn(dp, "\n")] = 0;

            char dn[MAX_PHONE_LENGTH];
            if (Normalize_Phone(dp, dn, sizeof(dn)) < 0) {
                puts("Invalid phone format!");
                continue;
            }
            if (hash_table_delete(table, dn)) {
                printf("Deleted %s\n", dn);
                Logging_Write(LOG_INFO, "Admin deleted record: %s", dn);
                csv_write_data(DB_FILE, table);
            } else {
                puts("Record not found!");
            }

        } else if (choice == 4) {
            view_pending_reports(table, nodes);
        } else if (choice == 5) {
            analyze_number(table, nodes);
        } else {
            // 🔧 Notify invalid choice instead of exiting immediately
            puts("Invalid selection. Please choose between 1 and 6.");
            continue;
        }
    }
}