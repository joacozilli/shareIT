#include "utils.h"
#include "str.h"
#include <string.h>
#include <stdio.h>

pthread_mutex_t eprintf_mutex = PTHREAD_MUTEX_INITIALIZER;


int safe_strerror1(char* errbuff, size_t bufflen) {
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

Array parse_input(char* input, char* delim) {
    char* token = strtok(input, delim);
    if (token == NULL)
        return NULL;

    Array arr = array_create(5, str_copy, str_delete, str_print);
    array_add(arr, token);
    while ((token = strtok(NULL, delim)))
        array_add(arr, token);

    return arr;
}