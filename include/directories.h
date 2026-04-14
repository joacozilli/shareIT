#ifndef __DIRECTORIES_H__
#define __DIRECTORIES_H__

#include "files.h"
#include "avl_tree.h"
#include "array.h"

struct _directory {

    char* name;
    // avl of type directory
    AVL subdirectories;

    // avl of type file_info
    AVL files;
};
typedef struct _directory* directory;



struct _shared_files {

    // avl of type directory
    AVL directories;

    pthread_mutex_t mutex;
};
typedef struct _shared_files* shared_files;


struct _read_directory_context {
    AVL parent_directory_subdirs;
};
typedef struct _read_directory_context* read_directory_context;

/////////////////////// functions to be used in data structures ///////////////////////

// identity function to be used as copy function.
void* directory_id(void* dir);

// directory comparison is done by name comparison
int directory_compare(void* dir1, void* dir2);

void directory_delete(void* dir);

void directory_print(void* dir);

/**
 * Read the directory, create corresponding directory structure
 */
void* read_directory(void* dir, void* context);
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Return new empty directory with the given name. Return NULL on error.
 */
directory directory_create(char* name);

/**
 * Build and return a shared_files struct where all shared files's name, size and path
 * are stored while preserving the directories's structure.
 */
shared_files get_shared_files(Array dirs);

#endif