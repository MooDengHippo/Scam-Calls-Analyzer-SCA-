#ifndef UTILS_H
#define UTILS_H

/* Need size_t */
#include <stddef.h>
#include <ctype.h>
#include <string.h>

#define MAX_PHONE_LENGTH 32

/* Function prototypes */
int  normalize_phone(const char *raw, char *out_buf, size_t out_sz);
int  get_country_code(const char *norm, char *dst, size_t dst_sz);
int  is_sea_country(const char *norm);

#endif /* UTILS_H */
