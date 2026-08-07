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

    Array dirs_paths;

    /**
     * write operation occurs every t seconds.
     * 
     * Consider a counter C and flag F with their respective mutex.
     * Every t seconds, no more readers are allowed.
     * INIT:
     *  C := 0
     *  F := 0
     * 
     * READER:
     *  START
     *  lock(F_mutex)
     *  While (F == 1):
     *      cond_wait(no_writer, F_mutex)
     * 
     *  lock(C_mutex)
     *  C := C+1
     *  free(C_mutex)
     * 
     *  free(F_mutex)
     * 
     *  read...
     * 
     *  lock(C_mutex)
     *  C := C-1
     *  if (C == 0):
     *      cond_signal(no_readers)
     *  free(C_mutex)
     *  END
     * 
     * 
     * WRITER:
     *  START
     *  sleep(t)
     *  lock(F_mutex)
     *  F := 1
     *  free(F_mutex)
     * 
     *  lock(C_mutex)
     *  while(C > 0):
     *      cond_wait(no_readers, C_mutex);
     *  free(C_mutex)
     * 
     *  write...
     * 
     *  lock(F_mutex)
     *  F := 0
     *  cond_broadcast(no_writer)
     *  free(F_mutex)
     *  END
     * 
     */
    
    int readers_counter;
    pthread_mutex_t readers_counter_mutex;
    pthread_cond_t no_readers;

    /**
     * 0 --> allowed to read
     * 1 --> not allowed to read (writer took lock)
     */
    int writer_flag;
    pthread_mutex_t writer_flag_mutex;
    pthread_cond_t no_writer;

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
///////////////////////////////////////////////////////////////////////////////////////

/**
 * Return new empty directory with the given name. Return NULL on error.
 */
directory directory_create(char* name);

/**
 * Read the directory and create corresponding directory structure
 */
void* read_directory(void* dir, void* context);

/**
 * Build and return a shared_files struct where all shared files's name, size and path
 * are stored while preserving the directories's structure.
 */
shared_files get_shared_files(Array dirs);


/**
 * Update shared files structure rereading the shared directories.
 */
void update_shared_files(shared_files sf);

#endif