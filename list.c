//
// Created by Theodore Ahlfeld on 9/20/15.
//

#include <stdlib.h>
#include <string.h>
#include "list.h"

void traverse_list(List *list, void (*f)(void *))
{
    Node *node = list->head;
    while(node) {
        f(node->data);
        node = node->next;
    }
}

void remove_all_nodes(List *list)
{
    Node *node = list->head;
    while(node) {
        Node *tmpNode = node;
        node = node->next;
        free(tmpNode);
    }
    init_list(list);

}

void *remove_node(List *lst, Node *node)
{
    if(node == NULL) {                  // empty list
        return NULL;
    }
    Node *tmp;
    void *data = node->data;
    if(node->next) {                    // node is not tail
        if(node->next == lst->tail) {
            lst->tail = node;
        }
        memcpy(node, node->next, sizeof(Node));
        tmp = node->next;
    } else if (node->prev) {            // node is tail
        (node->prev)->next = NULL;
        lst->tail = node->prev;
        tmp = node;
        node = NULL;
    } else {                            // node is only element in list
        lst->head = NULL;
        lst->tail = NULL;
        tmp = node;
        node = NULL;
    }
    free(tmp);
    return data;
}

Node *add_end(List *list, void *data)
{
    if(list->head == NULL) {
        return add_front(list, data);
    }
    Node *new_node = (struct Node *)malloc(sizeof(Node));

    if(!new_node)
        return NULL;
    new_node->data = data;
    new_node->next = NULL;
    (list->tail)->prev = new_node;
    list->tail = new_node;
    return new_node;
}

Node *add_front(List *list, void *data)
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    /* Only performs operation  */
    if(!new_node)
        return NULL;
    new_node->data = data;
    if(list->head) {
        (list->head)->prev = new_node;
    }
    new_node->next = list->head;
    new_node->prev = NULL;
    list->head = new_node;
    if(list->tail == NULL) {
        list->tail = new_node;
    }
    return new_node;
}

Node *find_node(List *list, const void *dataSought,
                      int (*compar)(const void *, const void *))
{
    struct Node *tmpNode = list->head;
    while(tmpNode) {
        if(compar(dataSought, tmpNode->data) == 0)
            return tmpNode;
        tmpNode = tmpNode->next;
    }
    return NULL;
}
