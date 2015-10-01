//
// Created by Theodore Ahlfeld on 9/20/15.
//

#ifndef __IRC_LIST_H
#define __IRC_LIST_H

typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct List {
    Node *head;
    Node *tail;
} List;

static inline void init_list(List *list)
{
    list->head = NULL;
    list->tail = NULL;
}

void traverse_list(List *list, void (*f)(void *));
void remove_all_nodes(List *list);
void *remove_node(List *list, Node *node);
Node *add_end(List *list, void *data);
Node *add_front(List *list, void *data);
Node *find_node(List *list, const void *dataSought,
                int (*compar)(const void *, const void *));

#endif //HW1_LIST_H
