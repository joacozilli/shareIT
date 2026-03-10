#include "avl_tree.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


AVL avl_create(functionCopy copy, functionCompareOrd cmp, functionDelete del, functionPrint print) {
    if (!copy || !cmp || !del)
        return NULL;
    
    AVL tree = malloc(sizeof(struct _AVL));
    tree->root = NULL;
    tree->copy = copy;
    tree->cmp = cmp;
    tree->del = del;
    tree->print = print;

    return tree;
}


//===================================== Auxiliar functions ==============================================

int size(AVLNode T) {
    if (!T)
        return 0;
    return T->size;
}

int height(AVLNode T) {
    if (!T)
        return 0;
    return T->height;
}

void set_size(AVLNode T) {
    T->size = 1 + size(T->left) + size(T->right);
}

int max(int a, int b) {
    return a > b ? a : b;
}

void set_height(AVLNode T) {
    T->height = 1 + max(height(T->left), height(T->right));
}

int balance_factor(AVLNode T) {
    return abs(height(T->left) - height(T->right));
}


//=======================================================================================================
//======================================== Rotations ====================================================

AVLNode simple_rot_left(AVLNode T) {
    AVLNode newroot = T->left;
    T->left = newroot->right;
    newroot->right = T;
    set_height(T);
    set_size(T);
    set_height(newroot);
    set_size(newroot);
    return newroot;
}

AVLNode simple_rot_right(AVLNode T) {
    AVLNode newroot = T->right;
    T->right = newroot->left;
    newroot->left = T;
    set_height(T);
    set_size(T);
    set_height(newroot);
    set_size(newroot);
    return newroot;
}

AVLNode double_rot_LR(AVLNode T) {
    T->left = simple_rot_right(T->left);
    return simple_rot_left(T);
}

AVLNode double_rot_RL(AVLNode T) {
    T->right = simple_rot_left(T->right);
    return simple_rot_right(T);
}
//=======================================================================================================

AVLNode balance_left(AVLNode T) {
    if (height(T->left->left) > height(T->left->right))
        return simple_rot_left(T);
    return double_rot_LR(T);
}

AVLNode balance_right(AVLNode T) {
    if (height(T->right->right) > height(T->right->left))
        return simple_rot_right(T);
    return double_rot_RL(T);
}


AVLNode insert_aux(AVLNode T, void* value, functionCompareOrd cmp, functionCopy copy, functionDelete del) {
    if (T == NULL) {
        AVLNode new = malloc(sizeof(struct _AVLNode));
        new->value = copy(value);
        new->left = NULL;
        new->right = NULL;
        new->size = 1;
        new->height = 1;
        return new;
    }
    int res = cmp(T->value, value);
    if (res > 0) {
        T->size++;
        T->left = insert_aux(T->left, value, cmp, copy, del);
        set_height(T);
        if (balance_factor(T) > 1)
            return balance_left(T);
        return T;
    }

    else if (res < 0) {
        T->size++;
        T->right = insert_aux(T->right, value, cmp, copy, del);
        set_height(T);
        if (balance_factor(T) > 1)
            return balance_right(T);
        return T;
    }

    else {
        del(T->value);
        T->value = copy(value);
        return T;
    }
}

void avl_insert(AVL tree, void* value) {
    if (!tree) {
        eprintf("avl tree given is NULL in %s\n", __func__);
        return;
    }
    tree->root = insert_aux(tree->root, value, tree->cmp, tree->copy, tree->del);
}


int avl_search_aux(AVLNode T, void* value, functionCompareOrd cmp) {
    if (T == NULL)
        return 0;

    int res = cmp(T->value, value);

    if (res == 0)
        return 1;
    
    if (res > 0)
        return avl_search_aux(T->left, value, cmp);

    if (res < 0)
        return avl_search_aux(T->right, value, cmp);
}

int avl_search(AVL tree, void* value) {
    if(!tree) {
        eprintf("avl tree given is NULL in %s\n", __func__);
        return -1;
    }
    return avl_search_aux(tree->root, value, tree->cmp);
}


void avl_delete(AVL tree, void* value) {
    return;
}



void destroy_aux(AVLNode T, functionDelete del) {
    if(T == NULL)
        return;   
    destroy_aux(T->left, del);
    destroy_aux(T->right, del);
    del(T->value);
    free(T);
}


void avl_destroy(AVL tree) {
    if (!tree)
        return;
    destroy_aux(tree->root, tree->del);
    free(tree);
}


void print_aux(AVLNode T, functionPrint print) {
    if(T == NULL)
        return;
    print_aux(T->left, print);
    print(T->value);
    print_aux(T->right, print);
}

void avl_print(AVL tree) {
    if (!tree) {
        eprintf("avl tree given is NULL in %s\n", __func__);
        return;
    }
    if (!tree->print)
        return;
    print_aux(tree->root, tree->print);
}

AVLNode map_aux(AVLNode T, functionMap f) {
    if (T == NULL)
        return NULL;
    T->left = map_aux(T->left, f);
    T->right = map_aux(T->right, f);
    void* temp = T->value;
    T->value = f(T->value);
    free(temp);
    return T;
}

void avl_map(AVL tree, functionMap f) {
    if (!tree) {
        eprintf("avl tree given is NULL in %s\n", __func__);
        return;
    }
    tree->root = map_aux(tree->root, f);
}