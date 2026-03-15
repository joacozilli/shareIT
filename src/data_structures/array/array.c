#include "array.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>


Array array_create(unsigned int capacity, functionCopy copy, functionDelete del, functionPrint print) {
    if (capacity <= 0 || !copy || !del) {

        return NULL;
    }
    Array arr = malloc(sizeof(struct _Array));

    arr->size = 0;
    arr->capacity = capacity;
    arr->copy = copy;
    arr->del = del;
    arr->print = print;
    arr->elems = malloc(sizeof(void*) * capacity);
    for (unsigned int i = 0; i < capacity; i++)
        arr->elems[i] = NULL;
    
    return arr;
}

void array_add(Array arr, void* value) {
    if (!arr) {
        log_error("array given is NULL");
        return;
    }
    if (!value) {
        log_error("value given is NULL");
        return;
    }
    arr->size++;
    if (arr->size > arr->capacity) {
        int inc = arr->capacity*2 < MAX_INCREMENT ? arr->capacity*2 : MAX_INCREMENT;
        int newCapacity = arr->capacity + inc;
        arr->elems = realloc(arr->elems, sizeof(void*) * newCapacity);
        arr->capacity = newCapacity;
    }
    arr->elems[arr->size-1] = arr->copy(value);
}

void* array_idx(Array arr, unsigned int i) {
    if (!arr) {
        log_error("array given is NULL");
        return NULL;
    }
    if (i >= arr->size) {
        log_error("index is larger than array size");
        return NULL;
    }
    return arr->elems[i];
}

void array_map(Array arr, functionMap f, void* context) {
    if (!arr) {
        log_error("array given is NULL");
        return;
    }   
    for (unsigned int i = 0; i < arr->size; i++)
        arr->elems[i] = f(arr->elems[i], context);
    return;
}

unsigned int array_size(Array arr) {
    if (!arr) {
        log_error("array given is NULL");
        return -1;
    }
    return arr->size;
}

void array_destroy(Array arr) {
    if (!arr) {
        log_error("array given is NULL");
        return;
    }

    for (unsigned int i = 0; i < arr->size; i++) 
        arr->del(arr->elems[i]);
    
    free(arr->elems);
    free(arr);
}


void array_print(Array arr) {
    if (!arr) {
        log_error("array given is NULL");
        return;
    }
    if(!arr->print)
        return;

    for (unsigned int i = 0; i < arr->size; i++)
        arr->print(arr->elems[i]);
}