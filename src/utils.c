#include "../include/utils.h"
#include <string.h>
#include <stdio.h>

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
