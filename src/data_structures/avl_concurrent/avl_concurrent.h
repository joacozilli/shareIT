#ifndef __AVL_CONCURRENT_H__
#define __AVL_CONCURRENT_H__
#include "avl_tree.h"
#include "funcs.h"
#include <pthread.h> // mutex

/**
 * Concurrent avl tree implementation. That is, thread safe avl tree.
 */

struct _conc_AVL {
    AVL tree;
    pthread_mutex_t mutex;
};
typedef struct _conc_AVL* conc_AVL;
/**
 * Create new empty concurrent avl tree with the given functions. Return NULL on error.
 * Print function is optional; must be NULL if not used.
 */
conc_AVL concurrent_avl_create(functionCopy copy, functionCompareOrd cmp, functionDelete del, functionPrint print);

/**
 * Insert value. If value already exists, update it.
 */
void concurrent_avl_insert(conc_AVL ctree, void* value);

/**
 * Return pointer to value if it is in tree, otherwise NULL.
 */
void* concurrent_avl_search(conc_AVL ctree, void* value);

/**
 * The same as avl_search, but the search criteria used is given as argument instead
 * of using the compare function specified during avl creation.
 */
void* concurrent_avl_search_by(conc_AVL ctree, void* value, functionCompareOrd cmp);

/**
 * Apply f to all values in concurrent avl tree. It is assumed f doesn't alter current order of values.
 * That is, for all a,b,c in the tree, if a < b < c then f(a) < f(b) < f(c). It is also assumed
 * f doesn't change the type of values.
 */
void concurrent_avl_map(conc_AVL ctree, functionMap f, void* context);

/**
 * Delete value from concurrent avl tree.
 */
void concurrent_avl_delete(conc_AVL ctree, void* value);

/**
 * Delete concurrent avl tree from memory.
 */
void concurrent_avl_destroy(conc_AVL ctree);

/**
 * Print concurrent avl tree in-order.
 */
void concurrent_avl_print(conc_AVL ctree);


#endif /* __AVL_CONCURRENT_H__ */