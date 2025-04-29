/* ───── MAIN — Scam Calls Analyzer ───── */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_manage.h"
#include "hash_map.h"
#include "graph.h"
#include "utils.h"
#include "cli_user.h"
#include "cli_admin.h"
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
/* ───── main program ───── */
int main(void){

    // 1) Core structures
    HashMap   *map   = hash_map_init();
    GraphNode *nodes[MAX_NODES] = {0};

    // 2) Load CSV
    const char *DB_FILE = "data/scam_numbers.csv";
    int rows = csv_read_data(DB_FILE, map, nodes);
    if(rows < 0){
        fprintf(stderr, "Error !!!: Failed to load data from %s\n", DB_FILE);
        hash_map_free(map);
        return EXIT_FAILURE;
    }

    // 3) Main loop
    while(1){
        print_main_menu();
        int choice = 0;
        if(scanf("%d%*c", &choice) != 1) break;

        if(choice == 1){
            user_mode(map, nodes);
        }else if(choice == 2){
            admin_mode(map, nodes);
        }else if(choice == 3){
            break;
        }else{
            puts("Invalid choice! Please select 1-3.");
        }
    }

    // 4) Persist data
    csv_write_data("data/scam_numbers.csv", map);

    // 5) Cleanup GraphNodes
    for(int i = 0; i < MAX_NODES; ++i){
        if(nodes[i]){
            graph_node_free(nodes[i]);
        }
    }

    hash_map_free(map);
    puts("See you later, Bye!!!");
    return 0;
}