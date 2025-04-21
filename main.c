/* main.c — Scam Calls Analyzer */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv_manage.h"
#include "hash_map.h"
#include "graph.h"
#include "utils.h"

/* ───── CLI helpers ───── */
static void print_main_menu(void)
{
    puts("========================================");
    puts("     Scam Calls Analyzer  (SCA)");
    puts("========================================");
    puts(" 1) User Mode  –  Check a phone number");
    puts(" 2) Admin Mode –  Manage database");
    puts(" 3) Exit");
    printf("\nEnter choice: ");
}

/* ASCII progress bar for risk score */
static void display_suspicious_score(float score)
{
    int bar = (int)(score * 20);   /* 0‒20 blocks */
    printf("\nSuspicious Score: %6.2f %%\n[", score * 100);
    for (int i = 0; i < 20; ++i) putchar(i < bar ? '#' : '-');
    puts("]");
    if      (score > 0.8)  puts("⚠  HIGH RISK!\n");
    else if (score > 0.5)  puts("⚠  MEDIUM RISK\n");
    else                   puts("✅  Low risk\n");
}

/* ───── main program ───── */
int main(void)
{
    /* 1) Core structures */
    HashMap   *map   = hash_map_init();
    GraphNode *nodes[MAX_NODES] = {0};

    /* 2) Load CSV */
    const char *DB_FILE = "data/scam_numbers.csv";
    int rows = csv_read_data(DB_FILE, map, nodes);
    if (rows < 0) {
        fprintf(stderr, "Error: Failed to load data from %s\n", DB_FILE);
        hash_map_free(map);
        return EXIT_FAILURE;
    }

    /* 3) Main loop */
    while (1)
    {
        print_main_menu();
        int choice = 0;
        if (scanf("%d%*c", &choice) != 1) break;   /* flush newline */

        if (choice == 1) {              /* ── USER MODE ── */
            char raw[64];
            printf("\nEnter phone (q to return): ");
            if (!fgets(raw, sizeof raw, stdin) || raw[0]=='q') { puts(""); continue; }

            char norm[MAX_PHONE_LENGTH];
            if (normalize_phone(raw, norm, sizeof norm) < 0) {
                puts("Invalid phone format\n"); continue;
            }

            ScamRecord *rec = hash_map_lookup(map, norm);
            if (rec) {
                printf("\n📌 Number found (reported %d times)\n", rec->report_count);
                display_suspicious_score(rec->suspicious_score);
            } else {
                if (!is_sea_country(norm)) {
                    puts("\n⚠  Foreign (non‑SEA) number – HIGH RISK!\n");
                } else {
                    puts("\n🔎 Not found – exploring relationship graph (BFS)\n");
                    graph_bfs(nodes, norm);
                }
            }

        } else if (choice == 2) {       /* ── ADMIN MODE ── */
            while (1) {
                puts("\n--- Admin Menu ---");
                puts(" 1) Add suspicious phone record");
                puts(" 2) Add relationship edge");
                puts(" 3) Back to main menu");
                printf("Select: ");
                int a = 0; if (scanf("%d%*c", &a)!=1) { a=3; }

                if (a == 1) {
                    char phone[64], risk_str[16], rep_str[16];
                    printf("Phone: "); fgets(phone, sizeof phone, stdin);
                    printf("Risk score (0‑1): "); fgets(risk_str, sizeof risk_str, stdin);
                    printf("Report count   : "); fgets(rep_str,  sizeof rep_str,  stdin);

                    char norm[MAX_PHONE_LENGTH];
                    if (normalize_phone(phone, norm, sizeof norm) < 0) { puts("Invalid phone"); continue; }
                    float sc = atof(risk_str);
                    if (sc < 0.0 || sc > 1.0) {
                        puts("Invalid risk score. Must be between 0 and 1.");
                        continue;
                    }
                    int   rc = atoi(rep_str);

                    hash_map_insert(map, norm, sc, rc);
                    printf("✅ Added %s\n", norm);

                } else if (a == 2) {
                    char p1[64], p2[64];
                    printf("Phone 1: "); fgets(p1,sizeof p1,stdin);
                    printf("Phone 2: "); fgets(p2,sizeof p2,stdin);

                    char n1[MAX_PHONE_LENGTH], n2[MAX_PHONE_LENGTH];
                    if (normalize_phone(p1,n1,sizeof n1)<0 || normalize_phone(p2,n2,sizeof n2)<0){
                        puts("One or both phones invalid"); continue;
                    }
                    graph_add_edge(nodes, n1, n2);
                    printf("✅ Linked %s ↔ %s\n", n1, n2);

                } else break;
            }

        } else if (choice == 3) {       /* ── EXIT ── */
            break;
        }
    }

    csv_write_data("data/scam_numbers.csv", map);

    /* Cleanup GraphNode array */
    for (int i = 0; i < MAX_NODES; ++i) {
        if (nodes[i]) {
            graph_node_free(nodes[i]);  // Assuming a function exists to free GraphNode
        }
    }

    hash_map_free(map);
    puts("Bye!");
    return 0;
}