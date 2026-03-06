#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#include "array.h"

// mutex for printing error messages
extern pthread_mutex_t eprintf_mutex;

// macro for printing errors on stderr. Thread safe.
#define eprintf(fmt,...) { \
    pthread_mutex_lock(&eprintf_mutex); \
    fprintf (stderr, fmt, __VA_ARGS__); \
    pthread_mutex_unlock(&eprintf_mutex); \
}

// macro for printing errors on stderr using errno. Thread safe.
#define errnoprintf(fmt, ...) { \
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

/**
 * Parse string input into tokens based on delim and return array with the tokens.
 * If there is no token, return NULL;
 */
Array parse_input(char* input, char* delim);



#endif