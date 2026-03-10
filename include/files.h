#ifndef __FILES_H__
#define __FILES_H__

#include <unistd.h>
#include "avl_concurrent.h"

struct _file_info {
    char* name;
    char* path;
    size_t size;
};
typedef struct _file_info* file_info;

/**
 * return deep copy of the file_info structure.
 */
void* file_info_copy(void* f);

/**
 * Return 0 if equals, < 0 if f1 < f2 and > 0 if f1 > f2.
 */
int file_info_compare(void* f1, void* f2);

/**
 * delete file_info from memory.
 */
void file_info_delete(void* f);

/**
 * print file_info to stdout.
 */
void file_info_print(void* f);


/**
 * Build a concurrent avl tree with all the files's name, path and size of the given directory.
 */
conc_AVL get_files(char* dir);

#endif