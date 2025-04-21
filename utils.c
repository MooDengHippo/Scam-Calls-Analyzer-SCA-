#include "utils.h"
#include <ctype.h>
#include <string.h>
#include "hash_map.h"
static const char *SEA_CODES[] = {
    "+66","+95","+855","+856","+60","+65",
    "+62","+63","+84","+673","+670"
};
static const int SEA_CODES_COUNT = sizeof(SEA_CODES)/sizeof(SEA_CODES[0]);

int normalize_phone(const char *raw, char *out, size_t out_sz){
    if(!raw || !out || out_sz < 5) return -1;
    char tmp[MAX_PHONE_LENGTH*2] = {0}; size_t j=0;
    if(*raw=='+'){ tmp[j++]='+'; raw++; }
    for(; *raw && j<sizeof(tmp)-1; raw++)
        if(isdigit((unsigned char)*raw)) tmp[j++] = *raw;
    tmp[j]='\0';
    if(tmp[0] != '+'){
        if(tmp[0]=='0'){
            char buf[MAX_PHONE_LENGTH*2] = "+66";
            strncat(buf,tmp+1,sizeof(buf)-4);
            strncpy(tmp,buf,sizeof(tmp));
        } else return -1;
    }
    if(strlen(tmp) >= out_sz) return -1;
    strcpy(out,tmp); return 0;
}

int get_country_code(const char *norm, char *dst, size_t dst_sz){
    if(!norm || norm[0] != '+' || dst_sz==0) return -1;
    size_t longest=0; const char *best=NULL;
    for(int i=0;i<SEA_CODES_COUNT;i++){
        size_t len = strlen(SEA_CODES[i]);
        if(strncmp(norm,SEA_CODES[i],len)==0 && len>longest){ longest=len; best=SEA_CODES[i]; }
    }
    if(!best){ if(dst_sz<4) return -1; strncpy(dst,norm,3); dst[3]='\0'; return 3; }
    if(longest >= dst_sz) return -1;
    strncpy(dst,best,longest); dst[longest]='\0'; return (int)longest;
}

int is_sea_country(const char *norm){
    char code[6];
    if(get_country_code(norm,code,sizeof(code))<0) return 0;
    for(int i=0;i<SEA_CODES_COUNT;i++) if(strcmp(code,SEA_CODES[i])==0) return 1;
    return 0;
}