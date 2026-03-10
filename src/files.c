#include "files.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>


void* file_info_copy(void* f) {
    file_info fi = (file_info) f;
    file_info copy = malloc(sizeof(struct _file_info));
    copy->name = malloc(sizeof(char) * (strlen(fi->name) + 1));
    strcpy(copy->name, fi->name);
    copy->path = malloc(sizeof(char) * (strlen(fi->path) + 1));
    strcpy(copy->path, fi->path);
    copy->size = fi->size;
    return copy;
}


int file_info_compare(void* f1, void* f2) {
    file_info ff1 = (file_info) f1;
    file_info ff2 = (file_info) f2;
    return strcmp(ff1->path, ff2->path);
}


void file_info_delete(void* f) {
    file_info ff = (file_info) f;
    free(ff->name);
    free(ff->path);
    free(ff);
}

void file_info_print(void* f) {
    file_info ff = (file_info) f;
    printf("file name: %s | file path: %s | file size: %lu bytes\n", ff->name,ff->path,ff->size);
}


conc_AVL get_files(char* dir) {
    
    struct dirent *dp;
    DIR* opened = opendir(dir);
    if (!opened) {
        errnoprintf("opendir in %s", __func__);
        return NULL;
    } 
    conc_AVL t = concurrent_avl_create(file_info_copy, file_info_compare, file_info_delete, file_info_print);

    while( (dp = readdir(opened)) != NULL) {
        char path[1000];
        sprintf(path, "%s/%s", dir, dp->d_name);
        struct stat st;
        if(stat(path, &st) < 0) {
            errnoprintf("stat in %s (regarding file %s)", __func__, dp->d_name);
            continue;
        }
        if (S_ISREG(st.st_mode)) {
            struct _file_info f;
            f.name = dp->d_name;
            f.path = path;
            f.size = st.st_size;
            concurrent_avl_insert(t, &f);
        }
    }
    return t;
}