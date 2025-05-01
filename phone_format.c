#include "phone_format.h"

static const char *SEA_CODES[] = {
    "+66", "+95", "+855", "+856", "+60", "+65",
    "+62", "+63", "+84", "+673", "+670"
};

static const int SEA_CODES_COUNT = sizeof(SEA_CODES) / sizeof(SEA_CODES[0]);
/*
 * Normalize Phone
 * -------------------------
 * Converts a raw phone number into a normalized international format.
 * If Thai number starts with '0', it converts to '+66'.
 */
int Normalize_Phone(const char *raw_input, char *normalized_out, size_t max_size){

    if (!raw_input || !normalized_out || max_size < 5) return -1;

    char buffer[MAX_PHONE_LENGTH * 2] = {0};
    size_t j = 0;

    if(*raw_input == '+'){
        buffer[j++] = '+';
        raw_input++;
    }

    for(; *raw_input && j < sizeof(buffer) - 1; raw_input++){
        if(isdigit((unsigned char)*raw_input))
            buffer[j++] = *raw_input;
    }
    buffer[j] = '\0';

    if(buffer[0] != '+'){
        if(buffer[0] == '0'){
            char tmp[MAX_PHONE_LENGTH * 2] = "+66";
            strncat(tmp, buffer + 1, sizeof(tmp) - 4);
            strncpy(buffer, tmp, sizeof(buffer));
        }else{
            return -1;
        }
    }

    if(strlen(buffer) < 10 || strlen(buffer) >= max_size) return -1;

    strcpy(normalized_out, buffer);
    return 0;

}
/*
 * Get Country Code
 * -------------------------
 * Extracts the calling code from a normalized number.
 * Example: "+66812345678" → "+66"
 */
int Get_Country_Code(const char *normalized, char *code_out, size_t code_out_size){

    if (!normalized || normalized[0] != '+' || code_out_size == 0) return -1;

    size_t longest = 0;
    const char *matched_code = NULL;

    for(int i = 0; i < SEA_CODES_COUNT; ++i){
        size_t len = strlen(SEA_CODES[i]);
        if(strncmp(normalized, SEA_CODES[i], len) == 0 && len > longest){
            longest = len;
            matched_code = SEA_CODES[i];
        }
    }

    if(!matched_code){
        if(code_out_size < 4) return -1;
        strncpy(code_out, normalized, 3);
        code_out[3] = '\0';
        return 3;
    }

    if(longest >= code_out_size) return -1;
    strncpy(code_out, matched_code, longest);
    code_out[longest] = '\0';
    return (int)longest;

}
/*
 * Is SEA Country
 * -------------------------
 * Checks if a phone number belongs to a SEA country.
 */
int Is_SEA_Country(const char *normalized){

    char country_code[6];
    if(Get_Country_Code(normalized, country_code, sizeof(country_code)) < 0) return 0;

    for(int i = 0; i < SEA_CODES_COUNT; ++i){
        if(strcmp(country_code, SEA_CODES[i]) == 0)
            return 1;
    }
    return 0;
    
}
/*
 * Calculate Score
 * -------------------------
 * Compute risk score based on region and number pattern
 * Used across csv_manage and admin module
 */
float calculate_score(const char *phone, int report_count, int neighbor_count){

    float base;
    if(!Is_SEA_Country(phone)) return 1.0f;

    if(strncmp(phone, "+66", 3) == 0){
        if(strncmp(phone, "+662", 4) == 0) base = 0.5f + 0.05f * report_count;
        else                                base = 0.1f + 0.05f * report_count;
    }else if(strncmp(phone, "+855", 4)==0 || strncmp(phone, "+95", 3)==0 || strncmp(phone, "+856", 4)==0)
        base = 0.7f + 0.05f * report_count;
    else
        base = 0.8f + 0.05f * report_count;

    float bonus = fminf(0.2f, 0.05f * neighbor_count);
    return fminf(1.0f, base + bonus);

}