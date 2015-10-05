/*
 * ============================================================================
 *
 *       Filename:  list.c
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

#include <stdlib.h>
#include <string.h>
#include "list.h"

/*
 * Traverses *list performing function (*f) on each node's data
 * struct List *list -> The list of nodes
 * void (*f)(void *) -> The function to perform on each node
 */
void traverse_list(List *list, void (*f)(void *))
{
    Node *node = list->head;
    while(node) {
        f(node->data);
        node = node->next;
    }
}

/*
 * Removes all nodes in *list
 * struct List *list -> List of nodes
 */
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

/*
 * Removes node from for list and returns the data
 * List *lst    -> The List to remove the node from
 * Node *node   -> The node to remove
 * returns the data from the removed node otherwise NULL
 */
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

/*
 * Adds a node at the end of *list containing *data
 * struct List *list -> The list of nodes to add the node in
 * void *data        -> The data for the new node to have
 * returns the new node if created otherwise NULL
 */
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

/*
 * Adds a node in front of *list containing *data
 * struct List *list -> The list of nodes to add the node in
 * void *data        -> The data for the new node to have
 * returns the new node if created otherwise NULL
 */
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

/*
 * Finds the node in *list with *dataSought using the *compar function
 * struct List *list       -> The list of nodes to search through
 * const void *dataSought  -> The data to find in the node
 * int (*compar)(const void *, const void)   ->
 *                The function to compare *dataSought and each node's data
 * return   -> returns NULL if node is not found otherwise returns the found node
 */
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
