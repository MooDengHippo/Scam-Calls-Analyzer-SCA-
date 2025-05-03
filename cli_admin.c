#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_admin.h"
#include "csv_manage.h"
#include "phone_format.h"
#include "logging.h"

// Convert score to description
static const char *get_risk_level_description(float score){

    if(score >= 0.81f) return "SEVERE";
    else if(score >= 0.61f) return "HIGH";
    else if(score >= 0.41f) return "MEDIUM";
    else if(score >= 0.21f) return "LOW";
    else return "VERY LOW";

}
// Show bar for risk score
static void display_suspicious_score(float score){

    int bar = (int)(score * 20);
    printf("\nSuspicious Score: %6.2f %%\n[", score * 100);
    for(int i = 0; i < 20; ++i)
        putchar(i < bar ? '#' : '-');
    puts("]\n");

}
// Risk info summary
static void display_user_risk_table(ScamRecord *rec){

    printf("=============================================\n");
    printf("|  Phone Number   | Reports |  Risk Level   |\n");
    printf("---------------------------------------------\n");
    printf("|  %-15s |   %-6d |  %-11s |\n",
        rec->phone,
        rec->report_count,
        get_risk_level_description(rec->suspicious_score));
    printf("=============================================\n\n");

}
// Reset visited flags in graph
static void reset_graph_visits(GraphNode *nodes[]){

    for(int i = 0; i < MAX_NODES; ++i)
        if(nodes[i]) nodes[i]->visited = 0;

}

// Print graph tree view
static void display_graph_ui(GraphNode *node, int level){

    if(!node || node->visited) return;
    node->visited = 1;

    for(int i = 0; i < level; ++i) printf("  ");
    printf("+-- %s\n", node->phone);

    for(int i = 0; i < node->neighbor_count; ++i)
        display_graph_ui(node->neighbors[i], level + 1);

}
// Pull and normalize phone
static int input_and_normalize_phone(char *prompt, char *normalized) {
    char raw[64];
    printf("%s", prompt);
    if (!fgets(raw, sizeof raw, stdin)) return -1;
    if (raw[0] == 'q' || raw[0] == 'Q') return -1;
    return Normalize_Phone(raw, normalized, MAX_PHONE_LENGTH);
}
// เพิ่ม Record หรือเชื่อม Graph
static void admin_add(HashTable *table, GraphNode *nodes[]){

    puts("\nAdd:");
    puts("1) Record");
    puts("2) Relationship Edge");
    printf("Enter choice (or 'q' to cancel): ");

    char input[16];
    fgets(input, sizeof input, stdin);
    if (input[0] == 'q' || input[0] == 'Q') return;

    if(input[0] == '1'){
        char norm[MAX_PHONE_LENGTH];
        if (input_and_normalize_phone("Enter phone number to add: ", norm) < 0){
            puts("\nInvalid phone format or cancelled.\n"); return;
        }

        ScamRecord *rec = hash_table_lookup(table, norm);
        if(rec){
            printf("\nRecord exists. Current report: %d\n", rec->report_count);
            printf("Increase report count? (y/n): ");
            char ans[8]; fgets(ans, sizeof ans, stdin);
            if(ans[0] == 'y' || ans[0] == 'Y'){
                rec->report_count++;
                rec->suspicious_score = calculate_score(norm, rec->report_count, 
                                        graph_get_node(nodes, norm)->neighbor_count);
                puts("\nReport increased!\n");
                Logging_Write(LOG_INFO, "Admin increased report for %s", norm);
            }
            return;
        }

        printf("Enter number of reports: ");
        int rep = 0;
        if(scanf("%d%*c", &rep) != 1 || rep < 1){
            puts("\nInvalid report count!\n"); while(getchar() != '\n'); return;
        }

        int neighbors = graph_get_node(nodes, norm)->neighbor_count;
        float score = calculate_score(norm, rep, neighbors);
        hash_table_insert(table, norm, score, rep);
        puts("\nRecord added!\n");
        Logging_Write(LOG_INFO, "Admin added record: %s", norm);

    }else if(input[0] == '2'){
        char normA[MAX_PHONE_LENGTH], normB[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter phone A: ", normA) < 0) return;
        if(input_and_normalize_phone("Enter phone B: ", normB) < 0) return;

        graph_add_edge(nodes, normA, normB);
        ScamRecord *r1 = hash_table_lookup(table, normA);
        ScamRecord *r2 = hash_table_lookup(table, normB);
        if(r1) r1->suspicious_score = calculate_score(normA, r1->report_count, graph_get_node(nodes, normA)->neighbor_count);
        if(r2) r2->suspicious_score = calculate_score(normB, r2->report_count, graph_get_node(nodes, normB)->neighbor_count);

        printf("\nEdge added. Increase risk score due to connection? (y/n): ");
        char ans[8]; fgets(ans, sizeof ans, stdin);
        if(ans[0] == 'y' || ans[0] == 'Y'){
            if(r1) r1->report_count++, Logging_Write(LOG_INFO, "Increased risk score for %s due to connection.", normA);
            if(r2) r2->report_count++, Logging_Write(LOG_INFO, "Increased risk score for %s due to connection.", normB);
        }
        puts("\nRelationship updated.\n");
        Logging_Write(LOG_INFO, "Admin added edge: %s <-> %s", normA, normB);

    }else{
        puts("\nInvalid choice.\n");
    }
}
// แก้ไข Record แบบยืนยัน
static void admin_edit(HashTable *table, GraphNode *nodes[]){
    char norm[MAX_PHONE_LENGTH];
    if(input_and_normalize_phone("Enter phone to edit: ", norm) < 0) return;

    ScamRecord *rec = hash_table_lookup(table, norm);
    if(!rec){ puts("\nNot found.\n"); return; }

    printf("\nCurrent Report = %d | Score = %.2f\n", rec->report_count, rec->suspicious_score);
    printf("Enter new report count (or 'q' to cancel): ");

    char buf[32];
    if(!fgets(buf, sizeof buf, stdin) || buf[0] == 'q' || buf[0] == 'Q') return;
    int val = atoi(buf);
    printf("Confirm changing report count to %d? (y/n): ", val);
    char ans[8]; fgets(ans, sizeof ans, stdin);
    if(ans[0] == 'y' || ans[0] == 'Y'){
        rec->report_count = val;
        rec->suspicious_score = calculate_score(norm, val, graph_get_node(nodes, norm)->neighbor_count);
        puts("\nRecord updated.\n");
        Logging_Write(LOG_INFO, "Admin edited record %s to report %d", norm, val);
    }
}
// ลบ Record หรือ Graph Edge พร้อมยืนยัน และ q
static void admin_delete(HashTable *table, GraphNode *nodes[]){
    puts("\nDelete:");
    puts("1) Record");
    puts("2) Relationship Edge");
    printf("Enter choice (or 'q' to cancel): ");
    char input[16];
    fgets(input, sizeof input, stdin);
    if(input[0] == 'q' || input[0] == 'Q') return;

    if(input[0] == '1'){
        char norm[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter phone to delete: ", norm) < 0) return;
        ScamRecord *rec = hash_table_lookup(table, norm);
        if(!rec){ puts("\nNumber not found.\n"); return; }
        printf("Confirm delete %s? (y/n): ", norm);
        char ans[8]; fgets(ans, sizeof ans, stdin);
        if(ans[0] == 'y' || ans[0] == 'Y'){
            hash_table_delete(table, norm);
            puts("\nRecord deleted.\n");
            Logging_Write(LOG_INFO, "Admin deleted record: %s", norm);
        }
    }else if(input[0] == '2'){
        char normA[MAX_PHONE_LENGTH], normB[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Phone A: ", normA) < 0) return;
        if(input_and_normalize_phone("Phone B: ", normB) < 0) return;
        printf("Confirm unlink %s <-> %s? (y/n): ", normA, normB);
        char ans[8]; fgets(ans, sizeof ans, stdin);
        if(ans[0] == 'y' || ans[0] == 'Y'){
            graph_remove_edge(nodes, normA, normB);
            puts("\nEdge removed.\n");
            Logging_Write(LOG_INFO, "Admin removed edge %s <-> %s", normA, normB);
        }
    }else puts("\nInvalid choice.\n");
}
// รับเบอร์ที่ pending มาเพิ่มลง record จริง พร้อมลบจาก pending
static void admin_view_pending(HashTable *table, GraphNode *nodes[]){
    FILE *fp = fopen("data/pending_reports.csv", "r");
    if(!fp){ puts("Cannot open pending_reports.csv\n"); return; }

    char phones[100][32];
    int count = 0;
    char line[128];
    puts("\nPending Reports:");
    while(fgets(line, sizeof line, fp) && count < 100){
        sscanf(line, "%[^,]", phones[count]);
        printf("%d) %s\n", count+1, phones[count]);
        count++;
    }
    fclose(fp);
    if(count == 0){ puts("No pending reports.\n"); return; }

    printf("Enter index to accept (or 'q' to cancel): ");
    char input[16];
    fgets(input, sizeof input, stdin);
    if(input[0] == 'q' || input[0] == 'Q') return;
    int idx = atoi(input);
    if(idx < 1 || idx > count){ puts("\nInvalid index.\n"); return; }

    char norm[MAX_PHONE_LENGTH];
    if(Normalize_Phone(phones[idx-1], norm, sizeof(norm)) < 0){ puts("Invalid format!\n"); return; }
    ScamRecord *rec = hash_table_lookup(table, norm);
    if(rec){
        rec->report_count++;
        rec->suspicious_score = calculate_score(norm, rec->report_count, graph_get_node(nodes, norm)->neighbor_count);
    }else{
        hash_table_insert(table, norm, 1.0, 1);
    }
    Logging_Write(LOG_INFO, "Admin accepted pending report: %s", norm);
    remove_pending_index(idx);
    puts("\nAccepted and added to main record.\n");
}
// Show full table and full graph links
static void admin_view_format(HashTable *table, GraphNode *nodes[]){
    puts("\nView:");
    puts("1) Record Table");
    puts("2) Graph Links");
    printf("Enter choice (or 'q' to cancel): ");
    char input[16]; fgets(input, sizeof input, stdin);
    if(input[0] == 'q' || input[0] == 'Q') return;

    if(input[0] == '1'){
        for(int i = 0; i < TABLE_SIZE; ++i){
            ScamRecord *rec = table->buckets[i];
            while(rec){
                display_user_risk_table(rec);
                rec = rec->next;
            }
        }
        Logging_Write(LOG_INFO, "Admin viewed full record table");
    }else if(input[0] == '2'){
        reset_graph_visits(nodes);
        for(int i = 0; i < MAX_NODES; ++i){
            if(nodes[i] && !nodes[i]->visited)
                display_graph_ui(nodes[i], 0);
        }
        Logging_Write(LOG_INFO, "Admin viewed graph links");
    }else puts("\nInvalid choice.\n");
}
// Analyze number like user
static void admin_analyze(HashTable *table, GraphNode *nodes[]){
    char norm[MAX_PHONE_LENGTH];
    if(input_and_normalize_phone("Enter phone to analyze: ", norm) < 0) return;
    ScamRecord *rec = hash_table_lookup(table, norm);
    if(rec){
        display_suspicious_score(rec->suspicious_score);
        display_user_risk_table(rec);
    } else puts("\nNot found.\n");
    reset_graph_visits(nodes);
    GraphNode *start = graph_get_node(nodes, norm);
    if(start && start->neighbor_count > 0){
        puts("\nConnected numbers:\n");
        display_graph_ui(start, 0);
    } else puts("\nNo links.\n");
    Logging_Write(LOG_INFO, "Admin analyzed number: %s", norm);
}
// Main admin loop
void admin_mode(HashTable *table, GraphNode *nodes[]){
    Logging_Write(LOG_INFO, "Admin Mode Entered");
    while(1){
        puts("\n=====================");
        puts("Admin Menu:");
        puts("1) Add Record/Edge");
        puts("2) Edit Record");
        puts("3) Delete Record/Edge");
        puts("4) View Pending Reports");
        puts("5) View Formatted Data");
        puts("6) Analyze Number");
        puts("7) Back to Main Menu");
        printf("Enter choice: ");
        char input[16]; fgets(input, sizeof input, stdin);
        if(input[0] == '1') admin_add(table, nodes);
        else if(input[0] == '2') admin_edit(table, nodes);
        else if(input[0] == '3') admin_delete(table, nodes);
        else if(input[0] == '4') admin_view_pending(table, nodes);
        else if(input[0] == '5') admin_view_format(table, nodes);
        else if(input[0] == '6') admin_analyze(table, nodes);
        else if(input[0] == '7') {
            Logging_Write(LOG_INFO, "Admin exited Admin Mode");
            break;
        }else puts("\nInvalid choice.\n");
    }
}