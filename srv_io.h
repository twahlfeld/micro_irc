/*
 * ============================================================================
 *
 *       Filename:  srv_io.h
 *
 *    Description:  Server's input and output and parser for client I/O
 *      Functions:
 *          void broadcast(const List *usr_lst, const char *msg, int self_sock)
 *              -> Prints msg to every socket except self_sock
 *          void private_msg(char *message, const User_Sock *src) ->
 *              Private messages a user, parse message to find destination
 *              User
 *          void client_cmd(List *usr_lst, Node *usr_node, char *msg) ->
 *              Parses the msg for the servers desired output
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

#ifndef __SRV_IO_H
#define __SRV_IO_H

#include "list.h"
#include "server.h"

/*
 * Broadcasts msg to all users except for theirself
 * const List *usr_lst -> The list of users to broadcast to
 * const char *msg     -> The message to broadcast
 * const int self_sock -> The originators socket to not broadcast to
 */
void broadcast(const List *usr_lst, const char *msg, const int self_sock);

/*
 * Broadcasts to a specified list of users
 * char *rol -> (rest of line) the clients msg without the broadcast user
 * int sock -> the sock of the original broadcaster
 */
void private_msg(char *rol, const User_Sock *src);


void client_cmd(List *usr_lst, Node *usr_node, char *msg);

#endif
