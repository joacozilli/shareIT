#include <stdio.h>
#include<stdlib.h>
#include <string.h>

#include "include/str.h"
#include "src/data_structures/array/array.h"
#include "src/data_structures/avl_tree/avl_tree.h"

// node structure would be something like this
// struct _NodeP2P {
//     char* name;
//     char ip[INET_ADDDRSTRLEN];
//     int port;
//     int counter;
// };

void* str_const(void* _str) {
    char* copy = malloc(sizeof(char) * (strlen("hola")+1));
    strcpy(copy, "hola");
    return copy;
}

int main() {

    AVL t = avl_create(str_copy, str_compare, str_delete, str_print);

    char buff[120];
    for (int i = 0; i < 10000; i++) {
        snprintf(buff, 120, "str%d",i);
        avl_insert(t, buff);
    }
    
    avl_map(t, str_const);

    //avl_print(t);

    printf("height: %d\nsize: %d\n", t->root->height, t->root->size);

    avl_destroy(t);

    return 0;
}