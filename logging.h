#ifndef LOGGING_H
#define LOGGING_H

typedef enum{

    LOG_INFO,
    LOG_WARN,
    LOG_ERROR

}LogLevel;
/*
 * Logging_Init
 * -------------------------
 * Initialize the log system by opening a log file.
 * Should be called once at program start.
 */
void Logging_Init(const char *log_file);
/*
 * Logging_Write
 * -------------------------
 * Write a formatted log message with timestamp and log level.
 * Works like printf.
 */
void Logging_Write(LogLevel level, const char *format, ...);
/*
 * Logging_Close
 * -------------------------
 * Close the log file safely. Call this at program exit.
 */
void Logging_Close(void);

#endif // LOGGING_H