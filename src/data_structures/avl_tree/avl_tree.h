#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

#include "data_structures/funcs.h"


struct _AVLNode {
    void* value;
    int height;
    int size;
    struct _AVLNode* left;
    struct _AVLNode* right;
};

typedef struct _AVLNode* AVLNode;

struct _AVL {
    AVLNode root;
    functionCopy copy;
    functionCompareOrd cmp;
    functionDelete del;
    functionPrint print;
};

typedef struct _AVL* AVL;



/**
 * Create new empty avl tree with the given functions. Return NULL on error.
 * Print function is optional; must be NULL if not used.
 */
AVL avl_create(functionCopy copy, functionCompareOrd cmp, functionDelete del, functionPrint print);

/**
 * Insert value. If value already exists, update it.
 */
void avl_insert(AVL tree, void* value);

/**
 * Apply f to all values in avl tree. It is assumed f doesn't alter current order of values.
 * That is, for all a,b,c in the tree, if a < b < c then f(a) < f(b) < (c)
 */
void avl_map(AVL tree, functionMap f);

/**
 * Delete value from avl tree.
 */
void avl_delete(AVL tree, void* value);

/**
 * Delete avl tree from memory.
 */
void avl_destroy(AVL tree);

/**
 * Print avl tree in-order.
 */
void avl_print(AVL tree);



#endif /* __AVL_TREE_H__ */