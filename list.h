/*
 * ============================================================================
 *
 *       Filename:  list.h
 *
 *    Description:  A simple IRC chat program
 *      Functions:
 *          void traverseList(struct List *list, void(*f)(void *))   ->
 *              Traverse through all nodes in *list perform function (*f)
 *          void removeAllNodes(struct List *list) ->
 *              Removes every node in *list
 *          void *remove_node(List *lst, Node *node) -> Removes node from lst
 *          Node *add_end(List *list, void *data) ->
 *              add a node at the end of list containing data
 *          Node *addFront(struct List *list, void *data)  ->
 *              Adds a new node in front of *list with *data
 *          Node *findNode(struct List *list, const void *dataSought,
 *           int(*compar)(const void *, const void *)) ->
 *              Find node in *list with *dataSought evaluated by *compar
 *
 *        Version:  1.0
 *        Created:  09/19/2015 14:53:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Theodore Ahlfeld (), twahlfeld@gmail.com
 *   Organization:
 *
 * ============================================================================
 */

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

/*
 * Traverses *list performing function (*f) on each node's data
 * struct List *list -> The list of nodes
 * void (*f)(void *) -> The function to perform on each node
 */
void traverse_list(List *list, void (*f)(void *));

/*
 * Removes all nodes in *list
 * struct List *list -> List of nodes
 */
void remove_all_nodes(List *list);

/*
 * Removes node from for list and returns the data
 * List *lst    -> The List to remove the node from
 * Node *node   -> The node to remove
 * returns the data from the removed node otherwise NULL
 */
void *remove_node(List *list, Node *node);

/*
 * Adds a node at the end of *list containing *data
 * struct List *list -> The list of nodes to add the node in
 * void *data        -> The data for the new node to have
 * returns the new node if created otherwise NULL
 */
Node *add_end(List *list, void *data);

/*
 * Adds a node in front of *list containing *data
 * struct List *list -> The list of nodes to add the node in
 * void *data        -> The data for the new node to have
 * returns the new node if created otherwise NULL
 */
Node *add_front(List *list, void *data);

/*
 * Finds the node in *list with *dataSought using the *compar function
 * struct List *list       -> The list of nodes to search through
 * const void *dataSought  -> The data to find in the node
 * int (*compar)(const void *, const void)   ->
 *                The function to compare *dataSought and each node's data
 * return   -> returns NULL if node is not found otherwise returns the found node
 */
Node *find_node(List *list, const void *dataSought,
                int (*compar)(const void *, const void *));

#endif //HW1_LIST_H
