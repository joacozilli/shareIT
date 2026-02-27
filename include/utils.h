#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>
#include <errno.h>
#include <stdio.h>

// macro for printing errors on stderr
#define eprintf(fmt,...) fprintf (stderr, fmt, __VA_ARGS__)

// macro for printing errors on stderr using errno
#define errnoprintf(fmt, ...) { \
    int _err = errno; \
    char _errbuf[256]; \
    safe_strerror(_errbuf, sizeof _errbuf); \
    eprintf(fmt, ##__VA_ARGS__); \
    eprintf(": %s\n", _errbuf); \
}

/**
 * thread-safe function to extract errno value (strerror uses internal buffer, 
 * and is only MT-safe in glibc >= 2.32). Store the string describing errno value
 * in the passed buffer. Return 0 on success, otherwise non zero.
 */
int safe_strerror(char* errbuff, size_t bufflen);


#endif