#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli_admin.h"
#include "phone_format.h"
#include "hash_table.h"
#include "graph.h"
/*
 * Admin Mode Main Handler
 * -------------------------
 * Allows admin to add records and edges.
 */
void admin_mode(HashTable *table, GraphNode *nodes[]){

    while(1){
        puts("\n--- Admin Menu ---");
        puts(" 1) Add suspicious phone record");
        puts(" 2) Add relationship edge");
        puts(" 3) Back to main menu");
        printf("Select: ");

        int choice = 0;
        if(scanf("%d%*c", &choice) != 1){
            choice = 3;
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
                continue;
            }
            float sc = atof(risk_str);
            if(sc < 0.0f || sc > 1.0f){
                puts("Invalid risk score! Must be between 0 and 1.");
                continue;
            }
            int rc = atoi(rep_str);

            hash_table_insert(table, norm, sc, rc);
            printf("Added %s\n", norm);

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
                continue;
            }
            graph_add_edge(nodes, n1, n2);
            printf("Linked %s ↔ %s\n", n1, n2);

        }else{
            break;
        }
    }
    
}