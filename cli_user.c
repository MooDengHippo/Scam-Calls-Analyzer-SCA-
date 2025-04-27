#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_user.h"
#include "utils.h"
#include "hash_map.h"
#include "graph.h"
// Private Helper: Display Suspicious Score 
static void display_suspicious_score(float score){

    int bar = (int)(score * 20); // 0‒20 blocks
    printf("\nSuspicious Score: %6.2f %%\n[", score * 100);
    for(int i = 0; i < 20; ++i) putchar(i < bar ? '#' : '-');
    puts("]");

    if      (score > 0.8)  puts("\u26a0  HIGH RISK!\n");
    else if (score > 0.5)  puts("\u26a0  MEDIUM RISK\n");
    else                   puts("\u2705  Low Risk\n");

}
// Private Helper: Display Relationship Graph
static void display_scam_graph(GraphNode *node, int level){

    if(!node) return;

    // Indentation
    for(int i = 0; i < level; i++) printf("  ");

    printf("\u250c──────────────\u2510\n");
    for(int i = 0; i < level; i++) printf("  ");
    printf("| %-12s |", node->phone);
    for(int i = 0; i < level; i++) printf("  ");
    printf("\u2514──────\u252c──────\u2518\n");

    for(int i = 0; i < node->neighbor_count; i++){
        display_scam_graph(node->neighbors[i], level + 1);
    }

}
// User Mode Main Handler 
void user_mode(HashMap *map, GraphNode *nodes[]){

    while(1){
        char raw[64];
        printf("\nEnter phone (q to return): ");
        if(!fgets(raw, sizeof raw, stdin) || raw[0]=='q'){
            puts("");
            break;
        }

        char norm[MAX_PHONE_LENGTH];
        if(Normalize_Phone(raw, norm, sizeof norm) < 0){
            puts("Invalid phone format!\n");
            continue;
        }

        ScamRecord *rec = hash_map_lookup(map, norm);
        if(rec){
            printf("\n\ud83d\udccc Number found (reported %d times)\n", rec->report_count);
            display_suspicious_score(rec->suspicious_score);
        }else{
            if(!Is_SEA_Country(norm)){
                puts("\n\u26a0  Foreign (non‑SEA) number – HIGH RISK!\n");
            }else{
                puts("\n\ud83d\udd0e Not found – exploring relationship graph:\n");
                GraphNode *start = graph_get_node(nodes, norm);
                if(start){
                    display_scam_graph(start, 0);
                }else{
                    puts("No relationships found!\n");
                }
            }
        }
    }

}