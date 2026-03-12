#include "avl_concurrent.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


conc_AVL concurrent_avl_create(functionCopy copy, functionCompareOrd cmp, functionDelete del, functionPrint print) {  
    AVL tree = avl_create(copy, cmp, del, print);
    if (!tree)
        return NULL;

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    conc_AVL new = malloc(sizeof(struct _conc_AVL));
    new->tree = tree;
    new->mutex = mutex;
    return new;
}


void concurrent_avl_insert(conc_AVL ctree, void* value) {
    if (!ctree) {
        eprintf("argument tree given is NULL in %s", __func__);
        return;
    }

    pthread_mutex_lock(&ctree->mutex);
    avl_insert(ctree->tree, value);
    pthread_mutex_unlock(&ctree->mutex);
}

void concurrent_avl_map(conc_AVL ctree, functionMap f, void* context) {
    if (!ctree) {
        eprintf("argument tree given is NULL in %s", __func__);
        return;
    }

    pthread_mutex_lock(&ctree->mutex);
    avl_map(ctree->tree, f, context);
    pthread_mutex_unlock(&ctree->mutex);
}


void* concurrent_avl_search(conc_AVL ctree, void* value) {
    if (!ctree) {
        eprintf("argument tree given is NULL in %s", __func__);
        return NULL;
    }

    pthread_mutex_lock(&ctree->mutex);
    void* res = avl_search(ctree->tree, value);
    pthread_mutex_unlock(&ctree->mutex);
    return res;

}

void* concurrent_avl_search_by(conc_AVL ctree, void* value, functionCompareOrd cmp) {
    if (!ctree) {
        eprintf("argument tree given is NULL in %s", __func__);
        return NULL;
    }

    pthread_mutex_lock(&ctree->mutex);
    void* res = avl_search_by(ctree->tree, value, cmp);
    pthread_mutex_unlock(&ctree->mutex);
    return res;   
}


void concurrent_avl_delete(conc_AVL ctree, void* value) {
    if (!ctree) {
        eprintf("argument tree given is NULL in %s", __func__);
        return;
    }

    pthread_mutex_lock(&ctree->mutex);
    avl_delete(ctree->tree, value);
    pthread_mutex_unlock(&ctree->mutex);
}


void concurrent_avl_destroy(conc_AVL ctree) {
    if(!ctree)
        return;

    pthread_mutex_lock(&ctree->mutex);
    avl_destroy(ctree->tree);
    pthread_mutex_unlock(&ctree->mutex);
    pthread_mutex_destroy(&ctree->mutex);
    free(ctree);
}

void concurrent_avl_print(conc_AVL ctree) {
    if (!ctree) {
        eprintf("argument tree given is NULL in %s", __func__);
        return;
    }

    pthread_mutex_lock(&ctree->mutex);
    avl_print(ctree->tree);
    pthread_mutex_unlock(&ctree->mutex);

}