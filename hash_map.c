#include "hash_map.h"
#include <stdlib.h>
#include <string.h>

static unsigned int hash_fn(const char *key){ unsigned h=0; while(*key) h = h*31 + (unsigned char)*key++; return h % TABLE_SIZE; }

HashMap *hash_map_init(void){ HashMap *m = calloc(1,sizeof(HashMap)); return m; }

void hash_map_insert(HashMap *m,const char *phone,float score,int rpt){
    unsigned idx = hash_fn(phone);
    ScamRecord *n = m->buckets[idx];
    while(n){ if(strcmp(n->phone,phone)==0){ n->suspicious_score = score; n->report_count = rpt; return;} n=n->next; }
    n = malloc(sizeof(ScamRecord));
    strncpy(n->phone,phone,MAX_PHONE_LENGTH); n->suspicious_score=score; n->report_count=rpt;
    n->next = m->buckets[idx]; m->buckets[idx]=n;
}

ScamRecord *hash_map_lookup(HashMap *m,const char *phone){
    unsigned idx = hash_fn(phone);
    for(ScamRecord *n=m->buckets[idx]; n; n=n->next)
        if(strcmp(n->phone,phone)==0) return n;
    return NULL;
}

void hash_map_free(HashMap *m){
    for(int i=0;i<TABLE_SIZE;i++){
        ScamRecord *n=m->buckets[i];
        while(n){ ScamRecord *tmp=n; n=n->next; free(tmp);} }
    free(m);
}