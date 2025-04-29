#ifndef LOGGING_H
#define LOGGING_H

typedef enum{

    LOG_INFO,
    LOG_WARN,
    LOG_ERROR
    
}LogLevel;

// Initialize logging system (call once)
void Logging_Init(const char *log_file);

// Log message with timestamp and level
void Logging_Write(LogLevel level, const char *format, ...);

// Close log file on shutdown
void Logging_Close(void);

#endif // LOGGING_H