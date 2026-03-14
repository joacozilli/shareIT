#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <pthread.h>

extern FILE *log_file;
extern pthread_mutex_t log_mutex;

/**
 * thread-safe function to extract errno value (strerror uses internal buffer, 
 * and is only MT-safe in glibc >= 2.32). Store the string describing errno value
 * in the passed buffer. Return 0 on success, otherwise non zero.
 */
int safe_strerror(char* errbuff, size_t bufflen);


void log_error_impl(const char *file, int line, const char *func,
                    const char *fmt, ...);

void log_info_impl(const char *file, int line, const char *func,
                   const char *fmt, ...);

void log_errno_impl(const char *file, int line, const char *func,
                    const char *fmt, ...);

#define log_error(fmt, ...) \
    log_error_impl(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define log_info(fmt, ...) \
    log_info_impl(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define log_errno(fmt, ...) \
    log_errno_impl(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)



#endif