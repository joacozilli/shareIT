#include "list.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


List list_create(functionCopy copy, functionCompareEq compare, functionDelete del, functionPrint print)
{
    if (!copy || !compare || !del)
        return NULL;
    
    List list = malloc(sizeof(struct _list));

    list->list = NULL;
    list->copy = copy;
    list->compare = compare;
    list->del = del;
    list->print = print;

    return list;
}

int list_is_empty(List list) {
    return list->list == NULL;
}

void list_add(List list, void* value) {
    if (!list) {
        eprintf("list given is NULL in %s\n", __func__);
        return;
    }
    Node new = malloc(sizeof(struct _Node));
    new->value = list->copy(value);
    new->next = list->list;
    list->list = new;
}


void* list_head(List list) {
    if (!list) {
        eprintf("list given is NULL in %s\n", __func__);
        return NULL;
    }

    if (!list->list)
        return NULL;

    void* head = list->list->value;
    list->list = list->list->next;

    return head;
}

int list_member(List list, void* value) {
    if (!list) {
        eprintf("list given is NULL in %s\n", __func__);
        return NULL;
    }

    for (Node temp = list->list; temp != NULL; temp = temp->next) {
        if (list->compare(temp->value, value) == 0)
            return 1;
    }
    return 0;
}


void list_remove(List list, void* value) {
    if (!list) {
        eprintf("list given is NULL in %s\n", __func__);
        return NULL;
    }

    // empty list
    if (!list->list)
        return NULL;
    
    // value is head
    if (list->compare(list->list->value, value) == 0) {
        Node temp = list->list;
        list->list = list->list->next;
        list->del(temp->value);
        free(temp);

    }

    Node temp = list->list->next;
    for (Node behind = list->list; temp != NULL; behind = behind->next) {
        if (list->compare(temp->value, value) == 0) {
            list->del(temp->value);
            behind->next = temp->next;
            free(temp);
        }

    }
}


void list_print(List list) {
    if (!list) {
        eprintf("list given is NULL in %s\n", __func__);
        return;
    }
    if (!list->print)
        return;
    for (Node temp = list->list; temp != NULL; temp = temp->next)
        list->print(temp->value);
}


void list_destroy(List list) {
    if (list) {
        Node node = list->list;
        while (node != NULL) {
            list->del(node->value);
            Node temp = node;
            node = node->next;
            free(temp);
        }
        free(list);
    }
}