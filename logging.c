#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "logging.h"

static FILE *log_fp = NULL;
void Logging_Init(const char *log_file){

    log_fp = fopen(log_file, "a");
    if(!log_fp){
        fprintf(stderr, "Failed to open log file: %s\n", log_file);
        exit(EXIT_FAILURE);
    }

}

void Logging_Write(LogLevel level, const char *format, ...){

    if(!log_fp) return;

    time_t raw = time(NULL);
    struct tm *t = localtime(&raw);

    fprintf(log_fp, "[%04d-%02d-%02d %02d:%02d:%02d] ",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    switch(level){
        case LOG_INFO:  fputs("[INFO ] ", log_fp); break;
        case LOG_WARN:  fputs("[WARN ] ", log_fp); break;
        case LOG_ERROR: fputs("[ERROR] ", log_fp); break;
        default:        fputs("[UNKWN] ", log_fp); break;
    }

    va_list args;
    va_start(args, format);
    vfprintf(log_fp, format, args);
    va_end(args);

    fputc('\n', log_fp);
    fflush(log_fp);

}

void Logging_Close(void){

    if(log_fp){
        fclose(log_fp);
        log_fp = NULL;
    }

}