/* ───── MAIN — Scam Calls Analyzer ───── */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_manage.h"
#include "hash_table.h"
#include "graph.h"
#include "phone_format.h"
#include "cli_user.h"
#include "cli_admin.h"
#include "logging.h"

/* ───── CLI Options ───── */
static void print_main_menu(void){

    puts("========================================");
    puts("     Scam Calls Analyzer  (SCA)");
    puts("========================================");
    puts(" 1) User Mode  -  Check a phone number");
    puts(" 2) Admin Mode -  Manage database");
    puts(" 3) Exit");
    printf("\nEnter choice: ");

}

/* ───── Main Program ───── */
int main(void){

    // 1) Core structures
    HashTable *table   = hash_table_init();
    GraphNode *nodes[MAX_NODES] = {0};

    // 2) Init logging
    Logging_Init("data/app.log");
    Logging_Write(LOG_INFO, "Program started");

    // 3) Load CSV
    const char *RECORD_FILE = "data/scam_numbers.csv";
    const char *EDGE_FILE   = "data/scam_edges.csv";
    int rows = csv_read_data(RECORD_FILE, EDGE_FILE, table, nodes);
    if(rows < 0){
        Logging_Write(LOG_ERROR, "Failed to load data from %s and %s", RECORD_FILE, EDGE_FILE);
        fprintf(stderr, "Error: Failed to load data from CSV files\n");
        hash_table_free(table);
        Logging_Close();
        return EXIT_FAILURE;
    }
    Logging_Write(LOG_INFO, "Loaded %d records from database", rows);

    // 4) Main loop
    while(1){
        print_main_menu();
        int choice = 0;
        if(scanf("%d%*c", &choice) != 1) break;

        if(choice == 1){
            Logging_Write(LOG_INFO, "User entered User Mode");
            user_mode(table, nodes);
        }else if(choice == 2){
            Logging_Write(LOG_INFO, "User entered Admin Mode");
            admin_mode(table, nodes);
        }else if(choice == 3) {
            Logging_Write(LOG_INFO, "User exited the program");
            break;
        }else{
            puts("Invalid choice! Please select 1-3.");
        }
    }

    // 5) Persist data to CSV
    if(csv_write_data(RECORD_FILE, table) == 0){
        Logging_Write(LOG_INFO, "Scam records saved to CSV");
    }
    if(csv_write_edges(EDGE_FILE, nodes) == 0){
        Logging_Write(LOG_INFO, "Graph relationships saved to CSV");
    }

    // 6) Cleanup GraphNodes
    for(int i = 0; i < MAX_NODES; ++i){
        if(nodes[i]){
            graph_node_free(nodes[i]);
        }
    }

    hash_table_free(table);
    Logging_Write(LOG_INFO, "Program exited cleanly!");
    Logging_Close();
    puts("See you later, Bye!!!");
    return 0;
    
}