#include "csv_manage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "hash_map.h"


static char *trim(char *s){ while(*s==' '||*s=='\t' || *s=='\n' || *s=='\r') s++; char *e=s+strlen(s)-1; while(e>s && (*e==' '||*e=='\t'||*e=='\n' || *e=='\r')) *e--='\0'; return s; }

int csv_read_data(const char *fname, HashMap *map, GraphNode *nodes[]){
    FILE *fp=fopen(fname,"r"); if(!fp){ perror("csv open"); return -1; }
    char line[256]; int cnt=0;
    while(fgets(line,sizeof(line),fp)){
        char *tok=strtok(line,","); if(!tok) continue; tok=trim(tok);
        if(*tok=='#') continue; // comment
        if(strcmp(tok,"R")==0){
            char *p=strtok(NULL,","), *score=strtok(NULL,","), *rep=strtok(NULL,",");
            if(!p||!score) continue;
            p=trim(p); score=trim(score);
            char norm[MAX_PHONE_LENGTH]; if(normalize_phone(p,norm,sizeof(norm))<0) continue;
            float sc=atof(score); int rc=rep?atoi(trim(rep)):1;
            // If phone NOT Thai & in SEA -> +bonus suspicion, if non‑SEA even higher
            int is_sea=is_sea_country(norm);
            if(!is_sea) sc = sc<0.5 ? 0.5f : sc; // bump risk of non‑SEA
            hash_map_insert(map,norm,sc,rc);
            cnt++;
        } else if(strcmp(tok,"E")==0){
            char *a=strtok(NULL,","), *b=strtok(NULL,","); if(!a||!b) continue;
            a=trim(a); b=trim(b);
            char na[MAX_PHONE_LENGTH], nb[MAX_PHONE_LENGTH];
            if(normalize_phone(a,na,sizeof(na))<0 || normalize_phone(b,nb,sizeof(nb))<0) continue;
            graph_add_edge(nodes,na,nb); cnt++; }
    }
    
    fclose(fp);
    return cnt;
}

int csv_write_data(const char *fname, HashMap *map){
    FILE *fp = fopen(fname, "w");
    if(!fp){ 
    perror("csv write"); 
    return -1; 
    }

    for(int i = 0; i < TABLE_SIZE; ++i){
        ScamRecord *rec = map->buckets[i];
        while(rec){
            fprintf(fp, "R,%s,%.2f,%d\n", rec->phone, rec->suspicious_score, rec->report_count);
            rec = rec->next;
        }
    }

    fclose(fp);
    return 0;
}