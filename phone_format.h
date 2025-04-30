#ifndef PHONE_FORMAT_H
#define PHONE_FORMAT_H
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#define MAX_PHONE_LENGTH 32
/*
 * Normalize Phone
 * -------------------------
 * Clean and format raw phone string into normalized form.
 * Example: "08-1234-5678" --> "+66812345678"
 * Returns 0 on success or -1 on failure
 */
int Normalize_Phone(const char *raw_input, char *normalized_out, size_t max_size);
/*
 * Get Country Code
 * -------------------------
 * Extract the country calling code from a normalized number.
 * Returns the length of the code or -1 on error.
 */
int Get_Country_Code(const char *normalized, char *code_out, size_t code_out_size);
/*
 * Is SEA Country
 * -------------------------
 * Check if a normalized phone number is from SEA region.
 * Returns 1 if yes, 0 otherwise.
 */
int Is_SEA_Country(const char *normalized);
/*
 * Calculate Score
 * -------------------------
 * Assign suspicious score based on phone pattern and report count
 */
float calculate_score(const char *phone, int report_count);

#endif // PHONE_FORMAT_H