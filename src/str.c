#include "str.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void* str_copy(void* str) {
    char* copy = malloc(sizeof(char) * (strlen((char*) str)+1));
    strcpy(copy, (char*) str);
    return copy;
}

int str_compare(void* str1, void* str2) {
    return strcmp(str1, str2);
}

void str_print(void* str) {
    printf("%s\n", (char*)str);
}

void str_delete(void* str) {
    free(str);
}