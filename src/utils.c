#include "utils.h"
#include "str.h"
#include <string.h>
#include <stdio.h>

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