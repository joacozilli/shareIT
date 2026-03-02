#ifndef __LIST_H__
#define __LIST_H__

#include "funcs.h"


/**
 * Simple linked list implementation.
 */

struct _Node {
    void* value;
    struct _Node* next;
};
typedef struct _Node* Node;

struct _list {
    Node list;

    functionCopy copy;
    functionCompareEq compare;
    functionDelete del;
    functionPrint print;
};
typedef struct _list* List;



/**
 * Create new empty list with the given functions. Return NULL on error.
 * Print function is optional; must be NULL if not used.
 */
List list_create(functionCopy copy, functionCompareEq compare, functionDelete del, functionPrint print);

/**
 * Return 1 if list is empty, otherwise 0.
 */
int list_is_empty(List list);

/**
 * Add value to head of list.
 */
void list_add(List list, void* value);

/**
 * Remove and return head of list. Return NULL if list is empty.
 */
void* list_head(List list);

/**
 * Return 1 if value is in the list, otherwise 0.
 */
int list_member(List list, void* value);

/**
 * Remove value from list. List remains unmodified if value isn't in it.
 * Only the first ocurrence is removed.
 */
void list_remove(List list, void* value);

/**
 * Print list.
 */
void list_print(List list);

/**
 * Delete list from memory.
 */
void list_destroy(List list);


#endif /* __LIST_H__ */