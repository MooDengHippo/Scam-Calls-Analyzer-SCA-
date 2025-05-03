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
// BFS view format
void bfs_print_group(GraphNode *nodes[], const char *start, int visited[]);
// Add record or link graph
static void admin_add(HashTable *table, GraphNode *nodes[]){

    puts("\nAdd:");
    puts("1) Record");
    puts("2) Relationship Edge");
    printf("Enter choice (or 'q' to cancel): ");

    char input[16];
    fgets(input, sizeof input, stdin);
    if(input[0] == 'q' || input[0] == 'Q') return;

    if(input[0] == '1'){
        char norm[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter phone number to add: ", norm) < 0){
            puts("\nInvalid phone format or cancelled.\n");
            return;
        }

        ScamRecord *rec = hash_table_lookup(table, norm);
        if(rec){
            printf("\nRecord exists. Current report: %d\n", rec->report_count);
            printf("Increase report count? (y/n): ");
            char ans[8];
            if(!fgets(ans, sizeof ans, stdin)) return;
            if(ans[0] == 'y' || ans[0] == 'Y'){
                rec->report_count++;
                rec->suspicious_score = calculate_score(norm, rec->report_count, graph_get_node(nodes, norm)->neighbor_count);
                puts("\nReport increased!\n");
                Logging_Write(LOG_INFO, "Admin increased report for %s", norm);
                csv_write_data("data/scam_numbers.csv", table);
            }else if(ans[0] != 'n' && ans[0] != 'N'){
                puts("\nInvalid input. Returning to main menu.\n");
            }
            return;
        }

        char buf[32];
        printf("Enter number of reports: ");
        if(!fgets(buf, sizeof buf, stdin)) return;
        if(buf[0] == 'q' || buf[0] == 'Q') return;

        int rep = atoi(buf);
        if(rep < 1){
            puts("\nInvalid report count!\n");
            return;
        }

        int neighbors = graph_get_node(nodes, norm)->neighbor_count;
        float score = calculate_score(norm, rep, neighbors);
        hash_table_insert(table, norm, score, rep);
        puts("\nRecord added!\n");
        Logging_Write(LOG_INFO, "Admin added record: %s", norm);
        csv_write_data("data/scam_numbers.csv", table);

    }else if(input[0] == '2'){
        char normA[MAX_PHONE_LENGTH], normB[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter phone A: ", normA) < 0) return;
        if(input_and_normalize_phone("Enter phone B: ", normB) < 0) return;

        int existed = already_connected(nodes, normA, normB);
        if(existed){
            puts("\nThis edge already exists. Returning to main menu.\n");
            return;
        }

        graph_add_edge(nodes, normA, normB);
        ScamRecord *r1 = hash_table_lookup(table, normA);
        ScamRecord *r2 = hash_table_lookup(table, normB);
        if(r1) r1->suspicious_score = calculate_score(normA, r1->report_count, graph_get_node(nodes, normA)->neighbor_count);
        if(r2) r2->suspicious_score = calculate_score(normB, r2->report_count, graph_get_node(nodes, normB)->neighbor_count);

        printf("\nEdge added. Increase risk score due to connection? (y/n): ");
        char ans[8];
        if(!fgets(ans, sizeof ans, stdin)) return;
        if(ans[0] == 'y' || ans[0] == 'Y'){
            if(r1) r1->report_count++;
            if(r2) r2->report_count++;
            Logging_Write(LOG_INFO, "Increased report count for new edge connection: %s <-> %s", normA, normB);
        }else if(ans[0] != 'n' && ans[0] != 'N'){
            puts("\nInvalid input. Returning to main menu.\n");
        }
        if(r1) r1->suspicious_score = calculate_score(normA, r1->report_count, graph_get_node(nodes, normA)->neighbor_count);
        if(r2) r2->suspicious_score = calculate_score(normB, r2->report_count, graph_get_node(nodes, normB)->neighbor_count);

        puts("\nRelationship updated.\n");
        Logging_Write(LOG_INFO, "Admin added edge: %s <-> %s", normA, normB);
        csv_write_data("data/scam_numbers.csv", table);
        csv_write_edges("data/scam_edges.csv", nodes);

    }else{
        puts("\nInvalid choice.\n");
    }
}
// Edit a Record or Relationship Edge in the system
static void admin_edit(HashTable *table, GraphNode *nodes[]){
    puts("\nEdit:");
    puts("1) Record");
    puts("2) Relationship Edge");
    printf("Enter choice (or 'q' to cancel): ");

    char choice[16];
    fgets(choice, sizeof choice, stdin);
    if(choice[0] == 'q' || choice[0] == 'Q') return;

    if(choice[0] == '1'){
        // Edit an existing scam record entry
        char norm[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter phone to edit: ", norm) < 0){
            puts("\nInvalid phone format or cancelled.\n");
            return;
        }

        ScamRecord *rec = hash_table_lookup(table, norm);
        if(!rec){
            puts("\nNumber not found.\n");
            return;
        }

        printf("\nCurrent info:\nPhone: %s\nReport: %d\nScore: %.2f\n", norm, rec->report_count, rec->suspicious_score);

        // Optionally change the phone number
        char new_phone[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter new phone (or 'q' to keep current): ", new_phone) < 0){
            strcpy(new_phone, norm); // keep current if cancelled
        }

        // Edit report count
        char buf[32];
        int new_report = rec->report_count;
        printf("Enter new report count (or 'q' to keep current): ");
        if(fgets(buf, sizeof buf, stdin)){
            if(buf[0] != 'q' && buf[0] != 'Q'){
                char *endptr;
                int temp = strtol(buf, &endptr, 10);
                if(endptr == buf || temp <= 0){
                    puts("\nInvalid report count. Edit cancelled.\n");
                    return;
                }
                new_report = temp;
            }
        }

        // Edit suspicious score
        float new_score = rec->suspicious_score;
        printf("Enter new suspicious score (or 'q' to keep current): ");
        if(fgets(buf, sizeof buf, stdin)){
            if(buf[0] != 'q' && buf[0] != 'Q'){
                char *endptr;
                float score = strtof(buf, &endptr);
                if(endptr == buf || score < 0.0f || score > 1.0f){
                    puts("\nInvalid score. Edit cancelled.\n");
                    return;
                }
                new_score = score;
            }
        }

        // Confirm changes
        printf("\nConfirm changes?\nNew Phone: %s\nNew Report: %d\nNew Score: %.2f\n(y/n): ", new_phone, new_report, new_score);
        char ans[8];
        if(!fgets(ans, sizeof ans, stdin)) return;
        if(ans[0] != 'y' && ans[0] != 'Y'){
            puts("\nEdit cancelled.\n");
            return;
        }

        // Apply changes to the table
        if(strcmp(norm, new_phone) != 0){
            ScamRecord *exist = hash_table_lookup(table, new_phone);
            if(exist){
                puts("\nNew phone number already exists. Edit aborted.\n");
                return;
            }
            hash_table_delete(table, norm);
            hash_table_insert(table, new_phone, new_score, new_report);
            Logging_Write(LOG_INFO, "Admin changed phone from %s to %s with report %d and score %.2f", norm, new_phone, new_report, new_score);
        }else{
            rec->report_count = new_report;
            rec->suspicious_score = new_score;
            Logging_Write(LOG_INFO, "Admin updated record %s to report %d and score %.2f", norm, new_report, new_score);
        }

        csv_write_data("data/scam_numbers.csv", table);
        puts("\nRecord updated.\n");

    }else if(choice[0] == '2'){
        // Edit an existing relationship edge by replacing it with a new connection

        char oldA[MAX_PHONE_LENGTH], oldB[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter current Phone A: ", oldA) < 0) return;
        if(input_and_normalize_phone("Enter current Phone B: ", oldB) < 0) return;

        // Check if the current edge exists in the graph
        if(!already_connected(nodes, oldA, oldB)){
            puts("\nNo existing edge found between the numbers. Please check the input.\n");
            return;
        }

        char newA[MAX_PHONE_LENGTH], newB[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter new Phone A: ", newA) < 0) return;
        if(input_and_normalize_phone("Enter new Phone B: ", newB) < 0) return;

        // Prevent replacing with an edge that already exists
        if(already_connected(nodes, newA, newB)){
            puts("\nNew edge already exists. Edit aborted.\n");
            return;
        }

        // Confirm the edge update with y/n and handle invalid input
        char confirm[8];
        while(1){
            printf("\nConfirm edge change?\nOld: %s <-> %s\nNew: %s <-> %s\n(y/n): ", oldA, oldB, newA, newB);
            if(!fgets(confirm, sizeof confirm, stdin)) return;
            if(confirm[0] == 'y' || confirm[0] == 'Y'){
                // Proceed with edge update after confirmation
                // Remove the old edge and add the new one, then log and write to CSV
                graph_remove_edge(nodes, oldA, oldB);
                graph_add_edge(nodes, newA, newB);
                puts("\nEdge updated.\n");
                Logging_Write(LOG_INFO, "Admin changed edge: %s <-> %s to %s <-> %s", oldA, oldB, newA, newB);
                csv_write_edges("data/scam_edges.csv", nodes);
                break;
            }else if(confirm[0] == 'n' || confirm[0] == 'N'){
                puts("\nEdit cancelled.\n");
                return;
            }else{
                puts("\nInvalid input. Please enter y or n.\n");
            }
        }

    }else{
        puts("\nInvalid choice.\n");
    }
}
// Delete a Record or Relationship Edge from the system
static void admin_delete(HashTable *table, GraphNode *nodes[]){
    puts("\nDelete:");
    puts("1) Record");
    puts("2) Relationship Edge");
    printf("Enter choice (or 'q' to cancel): ");

    char choice[16];
    fgets(choice, sizeof choice, stdin);
    if(choice[0] == 'q' || choice[0] == 'Q') return;

    if(choice[0] == '1'){
        // Delete a record from the hash table
        char norm[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter phone number to delete: ", norm) < 0){
            puts("\nInvalid phone format or cancelled.\n");
            return;
        }

        ScamRecord *rec = hash_table_lookup(table, norm);
        if(!rec){
            puts("\nNumber not found.\n");
            return;
        }

        // Confirm before deletion
        printf("\nAre you sure you want to delete record for %s? (y/n): ", norm);
        char confirm[8];
        if(!fgets(confirm, sizeof confirm, stdin)) return;
        if(confirm[0] == 'y' || confirm[0] == 'Y'){
            hash_table_delete(table, norm);
            puts("\nRecord deleted.\n");
            Logging_Write(LOG_INFO, "Admin deleted record: %s", norm);
            csv_write_data("data/scam_numbers.csv", table);
        }else if(confirm[0] == 'n' || confirm[0] == 'N'){
            puts("\nDeletion cancelled.\n");
        }else{
            puts("\nInvalid input. Delete aborted.\n");
        }

    }else if(choice[0] == '2'){
        // Delete a relationship edge from the graph
        char normA[MAX_PHONE_LENGTH], normB[MAX_PHONE_LENGTH];
        if(input_and_normalize_phone("Enter phone A: ", normA) < 0){
            puts("\nInvalid phone A.\n"); return;
        }
        if(input_and_normalize_phone("Enter phone B: ", normB) < 0){
            puts("\nInvalid phone B.\n"); return;
        }

        // Ensure edge exists exactly as entered
        if(!already_connected(nodes, normA, normB)){
            puts("\nNo edge found between the specified numbers.\n");
            return;
        }

        // Confirm before deletion
        printf("\nAre you sure you want to delete edge: %s <-> %s? (y/n): ", normA, normB);
        char confirm[8];
        if(!fgets(confirm, sizeof confirm, stdin)) return;
        if(confirm[0] == 'y' || confirm[0] == 'Y'){
            graph_remove_edge(nodes, normA, normB);
            puts("\nEdge deleted.\n");
            Logging_Write(LOG_INFO, "Admin deleted edge: %s <-> %s", normA, normB);
            csv_write_edges("data/scam_edges.csv", nodes);
        }else if(confirm[0] == 'n' || confirm[0] == 'N'){
            puts("\nDeletion cancelled.\n");
        }else{
            puts("\nInvalid input. Delete aborted.\n");
        }

    }else{
        puts("\nInvalid choice.\n");
    }
}
// View and accept pending reports from user-submitted CSV
static void admin_view_pending(HashTable *table, GraphNode *nodes[]){
    FILE *fp = fopen("data/pending_reports.csv", "r");
    if (!fp) {
        puts("\nCould not open pending report file.\n");
        return;
    }

    char line[128];
    int count = 0;
    char phones[100][MAX_PHONE_LENGTH];
    char times[100][32];

    // Read CSV lines with 2 columns: phone, timestamp
    while (fgets(line, sizeof line, fp)) {
        char *phone = strtok(line, ",");
        char *time = strtok(NULL, "\n");

        if (phone && time){
            strncpy(phones[count], phone, MAX_PHONE_LENGTH);
            phones[count][MAX_PHONE_LENGTH - 1] = '\0';
            strncpy(times[count], time, 32);
            times[count][31] = '\0';
            count++;
        }
    }
    fclose(fp);

    if (count == 0){
        puts("\nNo pending reports found.\n");
        return;
    }

    // Display in formatted table with index for selection
    printf("\n%-5s | %-20s | %-25s\n", "No.", "Phone Number", "Submitted At");
    puts("---------------------------------------------------------------");
    for(int i = 0; i < count; ++i){
        printf("%-5d | %-20s | %-25s\n", i+1, phones[i], times[i]);
    }

    // Ask for selection
    printf("\nEnter number to accept (or 'q' to cancel): ");
    char buf[16];
    fgets(buf, sizeof buf, stdin);
    if(buf[0] == 'q' || buf[0] == 'Q') return;

    int selection = atoi(buf);
    if(selection < 1 || selection > count){
        puts("\nInvalid choice.\n");
        return;
    }

    char *norm = phones[selection - 1];

    // Confirm acceptance
    char confirm[8];
    while(1){
        printf("\nConfirm accepting report for %s? (y/n): ", norm);
        if(!fgets(confirm, sizeof confirm, stdin)) return;
        if(confirm[0] == 'y' || confirm[0] == 'Y'){
            ScamRecord *rec = hash_table_lookup(table, norm);
            if(rec){
                rec->report_count++;
                rec->suspicious_score = calculate_score(norm, rec->report_count, graph_get_node(nodes, norm)->neighbor_count);
            }else{
                float score = calculate_score(norm, 1, graph_get_node(nodes, norm)->neighbor_count);
                hash_table_insert(table, norm, score, 1);
            }
            Logging_Write(LOG_INFO, "Accepted pending report for %s", norm);
            remove_pending_index(selection - 1);
            csv_write_data("data/scam_numbers.csv", table);
            puts("\nPending report accepted.\n");
            break;
        }else if(confirm[0] == 'n' || confirm[0] == 'N'){
            puts("\nAction cancelled.\n");
            break;
        }else{
            puts("\nInvalid input. Please enter y or n.\n");
        }
    }
}
// View formatted tables of all scam records and graph relationships
static void admin_view_format(HashTable *table, GraphNode *nodes[]){
    puts("\nView Format:");
    puts("1) Scam Record Table");
    puts("2) Scam Relationship Graph");
    printf("Enter choice (or 'q' to cancel): ");

    char input[8];
    fgets(input, sizeof input, stdin);
    if(input[0] == 'q' || input[0] == 'Q') return;

    if(input[0] == '1'){
        // Print all records in a single formatted table
        printf("\n===================================================================================\n");
        printf("| %-20s | %-10s | %-12s | %-20s |\n", "Phone Number", "Reports", "Risk Level", "Suspicious Score (%)");
        printf("-----------------------------------------------------------------------------------\n");
        for(int i = 0; i < TABLE_SIZE; i++){
            ScamRecord *rec = table->buckets[i];
            while(rec){
                const char *risk = (rec->suspicious_score >= 0.7) ? "HIGH" :
                                   (rec->suspicious_score >= 0.4) ? "MEDIUM" : "LOW";
                printf("| %-20s | %-10d | %-12s | %-20.2f |\n", rec->phone, rec->report_count, risk, rec->suspicious_score * 100);
                rec = rec->next;
            }
        }
        printf("===================================================================================\n");

    }else if(input[0] == '2'){
        reset_graph_visits(nodes);
        for(int i = 0; i < MAX_NODES; ++i){
            if(nodes[i] && !nodes[i]->visited)
                display_graph_ui(nodes[i], 0);
        }
        Logging_Write(LOG_INFO, "Admin viewed graph links");
    }else{
        puts("\nInvalid choice.\n");
    }
}
// Analyze number like user
static void admin_analyze(HashTable *table, GraphNode *nodes[]) {
    char norm[MAX_PHONE_LENGTH];
    if (input_and_normalize_phone("Enter phone to analyze: ", norm) < 0) {
        puts("\nInvalid phone format or cancelled.\n");
        return;
    }

    ScamRecord *rec = hash_table_lookup(table, norm);
    if (rec) {
        display_suspicious_score(rec->suspicious_score);
        display_user_risk_table(rec);
    } else {
        puts("\nNot found in database.\n");
    }

    reset_graph_visits(nodes);
    GraphNode *start = graph_get_node(nodes, norm);
    if (start && start->neighbor_count > 0) {
        puts("\nConnected numbers:\n");
        display_graph_ui(start, 0);
    } else {
        puts("\nNo links.\n");
    }

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

        char input[16];            
        fgets(input, sizeof input, stdin);
        int choice = atoi(input);

        switch(choice){
            case 1: admin_add(table, nodes); break;
            case 2: admin_edit(table, nodes); break;
            case 3: admin_delete(table, nodes); break;
            case 4: admin_view_pending(table, nodes); break;
            case 5: admin_view_format(table, nodes); break;
            case 6: admin_analyze(table, nodes); break;
            case 7:
                Logging_Write(LOG_INFO, "Admin exited Admin Mode");
                return;
            default:
                puts("\nInvalid choice.\n");
        }
    }
}