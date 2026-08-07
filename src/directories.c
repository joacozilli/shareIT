#include "directories.h"
#include "log.h"
#include "avl_tree.h"
#include "files.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>


void* directory_id(void* dir) {
    return dir;
}

int directory_compare(void* dir1, void* dir2) {
    return strcmp((char*) dir1, (char*) dir2);
}

void directory_delete(void* _dir) {
    directory dir = (directory) _dir;
    free(dir->name);
    avl_destroy(dir->subdirectories);
    avl_destroy(dir->files);
    free(dir);
}

void directory_print(void* _dir) {
    directory dir = (directory) _dir;
    printf("%s\n", dir->name);
    avl_print(dir->subdirectories);
    avl_print(dir->files);
}


directory directory_create(char* name) {
    directory newdir = malloc(sizeof(struct _directory));
    newdir->name = malloc(sizeof(char) * (strlen(name)+1));
    strcpy(newdir->name, name);
    
    newdir->subdirectories = avl_create(directory_id, directory_compare,
                                        directory_delete, directory_print);
    if (newdir->subdirectories == NULL) {
        free(newdir->name);
        free(newdir);
        return NULL;
    }
    
    newdir->files = avl_create(file_info_copy, file_info_compare,
                                file_info_delete, file_info_print);
    
    if (newdir->files == NULL) {
        avl_destroy(newdir->subdirectories);
        free(newdir->name);
        free(newdir);
        return NULL;
    }
    return newdir;
}

/**
 * Auxiliar function for read_directory(). Extract the directory's name from the
 * absolute path and write it in buff.
 */
void extract_dir_name(char* path, void* buff) {
    int len = strlen(path);
    int start = 0;
    for (int i = len-1; path[i] != '/' && i >= 0; i--, start = i);
    if (start == 0 && path[0] != '/') {
        strcpy(buff, path);
        return;
    }
    start++;
    strcpy(buff, path+start);
}

void* read_directory(void* dir, void* context) {

    read_directory_context cont = (read_directory_context) context;
    char* dir_path = (char*) dir;
    struct dirent *dp;
    DIR* opened = opendir(dir_path);
    if (!opened) {
        log_errno("error in opendir (related to directory %s)", dir_path);
        return dir;
    }

    char dir_name[256];
    extract_dir_name(dir_path, dir_name);
    directory newdir = directory_create(dir_name);

    while ((dp = readdir(opened)) != NULL) {

        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..") || dp->d_name[0] == '.')
            continue;

        char path[1000];
        sprintf(path, "%s/%s", dir_path, dp->d_name);
        struct stat st;
        if (stat(path, &st) < 0) {
            log_errno("error in stat (related to file %s)", path);
            continue;
        }

        if (S_ISREG(st.st_mode)) {
            struct _file_info f;
            f.name = dp->d_name;
            f.path = path;
            f.size = st.st_size;
            avl_insert(newdir->files, &f);
        }

        else if (S_ISDIR(st.st_mode)) {
            struct _read_directory_context newcontext;
            newcontext.parent_directory_subdirs = newdir->subdirectories;
            read_directory((void*) path, (void*) &newcontext);
        }
    }

    avl_insert(cont->parent_directory_subdirs, newdir);

    closedir(opened);
    return dir;
}


shared_files get_shared_files(Array dirs) {
    shared_files sf = malloc(sizeof(struct _shared_files));

    struct _read_directory_context context;
    context.parent_directory_subdirs = avl_create(directory_id, directory_compare,
                                            directory_delete, directory_print);

    array_map(dirs, read_directory, &context);
    sf->directories = context.parent_directory_subdirs;
    sf->readers_counter = 0;

    sf->dirs_paths = dirs;

    pthread_mutex_t readers_counter_mutex = PTHREAD_MUTEX_INITIALIZER;
    sf->readers_counter_mutex = readers_counter_mutex;

    pthread_cond_t no_readers = PTHREAD_COND_INITIALIZER;
    sf->no_readers = no_readers;

    sf->writer_flag = 0;

    pthread_mutex_t writer_flag_mutex = PTHREAD_MUTEX_INITIALIZER;
    sf->writer_flag_mutex = writer_flag_mutex;

    pthread_cond_t no_writer = PTHREAD_COND_INITIALIZER;
    sf->no_writer = no_writer;

    return sf;
}




void update_shared_files(shared_files sf) {

    // writer's lock
    pthread_mutex_lock(&sf->writer_flag_mutex);
    sf->writer_flag = 1;
    pthread_mutex_unlock(&sf->writer_flag_mutex);

    pthread_mutex_lock(&sf->readers_counter_mutex);
    while (sf->readers_counter > 0)
        pthread_cond_wait(&sf->no_readers, &sf->readers_counter_mutex);
    pthread_mutex_unlock(&sf->readers_counter_mutex);

    
    avl_destroy(sf->directories);
    struct _read_directory_context context;
    context.parent_directory_subdirs = avl_create(directory_id, directory_compare,
                                            directory_delete, directory_print);

    array_map(sf->dirs_paths, read_directory, &context);
    sf->directories = context.parent_directory_subdirs;

    // writer's unlock
    pthread_mutex_lock(&sf->writer_flag_mutex);
    sf->writer_flag = 0;
    pthread_cond_broadcast(&sf->no_writer);
    pthread_mutex_unlock(&sf->writer_flag_mutex);

    return;
}
