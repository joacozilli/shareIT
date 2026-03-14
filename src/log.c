#include "log.h"
#include <stdarg.h>
#include <errno.h>
#include <stdlib.h>

FILE *log_file = NULL;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;



int safe_strerror(char* errbuff, size_t bufflen) {
    #if defined(__GLIBC__) && defined(_GNU_SOURCE)
        /* GNU-specific */
        strerror_r(errno, errbuf, buflen);
        return 0;
    #else
        /* POSIX */
        int ret = strerror_r(errno, errbuff, bufflen);
        return ret;
    #endif
}

static void log_generic(const char *level,
                        const char *file,
                        int line,
                        const char *func,
                        const char *fmt,
                        va_list args)
{
    pthread_mutex_lock(&log_mutex);

    fprintf(log_file, "[%s] %s:%d %s: ", level, file, line, func);
    vfprintf(log_file, fmt, args);
    fprintf(log_file, "\n");
    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
}

void log_error_impl(const char *file, int line, const char *func, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_generic("ERROR", file, line, func, fmt, args);
    va_end(args);
}

void log_info_impl(const char *file, int line, const char *func, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_generic("INFO", file, line, func, fmt, args);
    va_end(args);
}

void log_errno_impl(const char *file, int line, const char *func, const char *fmt, ...) {
    int saved_errno = errno;
    char errbuf[256];
    safe_strerror(errbuf, sizeof errbuf);
    pthread_mutex_lock(&log_mutex);


    fprintf(log_file, "[ERROR] %s:%d %s: ", file, line, func);

    va_list args;
    va_start(args, fmt);
    vfprintf(log_file, fmt, args);
    va_end(args);

    fprintf(log_file, ": %s (errno=%d)\n", errbuf, saved_errno);
    fflush(log_file);

    pthread_mutex_unlock(&log_mutex);
}
