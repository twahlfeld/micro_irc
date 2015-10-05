/*
 * ============================================================================
 *
 *       Filename:  srv_io.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include "server.h"
#include "srv_io.h"

/* External global data from server.c */
extern size_t MAXBUF;
extern fd_set mstr_fds;
extern int serv_sock;
extern int timeout;

/*
 * Broadcasts msg to all users except for theirself
 * const List *usr_lst -> The list of users to broadcast to
 * const char *msg     -> The message to broadcast
 * const int self_sock -> The originators socket to not broadcast to
 */
void broadcast(const List *usr_lst, const char *msg, const int self_sock) {
    if (msg) {
        printf("%s", msg);
        Node *clnt_node = usr_lst->head;
        size_t len = strlen(msg);
        while (clnt_node) {
            User_Sock *usr_sock = (User_Sock *) (clnt_node->data);
            if (usr_sock) {
                int tmp_sock = usr_sock->sock;

                // send to everyone!
                if (FD_ISSET(tmp_sock, &mstr_fds)) {
                    // except the listener and ourselves
                    if (tmp_sock != serv_sock && tmp_sock != self_sock) {
                        if (send(tmp_sock, msg, len, 0) == -1) {
                            perror("send() failed");
                        }
                    }
                }
            }
            clnt_node = clnt_node->next;
        }
    }
}

/*
 * Broadcasts to a specified list of users
 * char *rol -> (rest of line) the clients msg without the broadcast user
 * int sock -> the sock of the original broadcaster
 */
void pvt_broadcast(char *rol, User_Sock *usr_src) {
    List pvt_usr_list;
    init_list(&pvt_usr_list);
    if (rol) {
        User_Sock *usr_sock;
        char *buf = strtok(rol, " \n"); // first user in rol
        char msg[MAXBUF];
        while (buf) { // ensures valid format
            if (strcasecmp(buf, "message") == 0) {
                buf = strtok(NULL, "\n");
                if (buf == NULL) goto cleanup;
                sprintf(msg, "%s: %s\n", usr_src->name, buf);
                broadcast(&pvt_usr_list, msg, usr_src->sock);
                break;
            } else {
                usr_sock = find_usr(buf);
                if (usr_sock) {
                    add_front(&pvt_usr_list, usr_sock);
                }
                buf = strtok(NULL, " \n");
            }
        }
    }

    cleanup:
    remove_all_nodes(&pvt_usr_list);
}

/*
 * Send a private message, destination is in rol
 * char *rol    -> (rest of line) include the user destination along with msg
 * const User_Sock *src -> The user that initiated the PM
 */
void private_msg(char *rol, const User_Sock *src) {
    char buf[MAXBUF];
    char *base;
    size_t len;

    if (rol == NULL) goto end;
    if ((base = strtok(rol, " \n")) == NULL) goto end;

    User_Sock *dst = find_usr(base);
    if (dst == NULL && (dst->name) == NULL) goto end;
    if ((base = strtok(NULL, "\n")) == NULL) goto end;
    fprintf(stderr, "%s->", src->name);
    fprintf(stderr, "%s:", dst->name);
    fprintf(stderr, "%s\n", base);
    printf("%s->%s: %s\n", src->name, dst->name, base);
    len = sprintf(buf, "%s: %s\n", src->name, base);
    send(dst->sock, buf, len, 0);

    end:
    return;
}

/*
 * Parses the clients output into procedure for the server
 * List *usr_lst    -> The list of users from the server
 * Node *usr_node   -> The node from usr_lst that initiated the command
 * char *msg        -> The message that the user submitted
 */
void client_cmd(List *usr_lst, Node *usr_node, char *msg) {
    char *cmd = strtok(msg, " \n"), *base;
    if(cmd == NULL) return;

    User_Sock *usr_sock = (User_Sock *) (usr_node->data);
    char *name = usr_sock->name;
    int sock = usr_sock->sock;
    Node *node_itr = usr_lst->head;
    char buf[MAXBUF];
    size_t len;

    if (strcasecmp(cmd, "quit") == 0 || strcasecmp(cmd, "logout") == 0) {
        remove_node(usr_lst, usr_node);
        kill_user(usr_sock, 0);
    } else if (strcasecmp(cmd, "broadcast") == 0) {
        if ((base = strtok(NULL, " \n")) == NULL) { // No further input
            goto end;
        } else {
            if (strcasecmp(base, "message") == 0) { // Full Broadcast
                sprintf(buf, "%s: %s\n", name, strtok(NULL, "\n"));
                broadcast(usr_lst, buf, sock);
            } else if (strcasecmp(base, "user") == 0) { // Private broadcast
                pvt_broadcast(strtok(NULL, "\n"), usr_sock);
            }
        }
    } else if (strcasecmp(cmd, "message") == 0) {   // Private message
        private_msg(strtok(NULL, "\n"), usr_sock);
    } else if (strcasecmp(cmd, "motd") == 0) {      // change motd
        strcpy(buf, strtok(NULL, "\n"));
        if (strlen(buf)) {
            set_motd(buf, name);
        } else {
            print_motd(sock);
        }
    } else if (strcasecmp(cmd, "timeout") == 0) {   // change timeout
        int new_timeout;
        strcpy(buf, strtok(NULL, " \n"));
        char *timeout_base = buf;
        new_timeout = (int) strtol(buf, &timeout_base, 10);
        if (*timeout_base != '\0') { // invalid format
            printf("%s failed to change timeout to %s\n", name, timeout_base);
            len = sprintf(buf, "SERVER: invalid input timeout\n");
            send(sock, buf, len, 0);
        } else {
            timeout = new_timeout;
            sprintf(buf, "%s changed timeout %d\n", name, timeout);
            broadcast(usr_lst, buf, -1);
        }
    } else if (strcasecmp(cmd, "whoelse") == 0) {   // whoelse
        char *active_client;
        len = sprintf(buf, "Current Users:\n");
        send(sock, buf, len, 0);
        while (node_itr) {
            if ((active_client = ((User_Sock *) (node_itr->data))->name)) {
                len = sprintf(buf, "\t%s\n", active_client);
                send(sock, buf, len, 0);
            }
            node_itr = node_itr->next;
        }
    } else if (strcasecmp(cmd, "wholast") == 0) {
        time_t rqst_time;
        strcpy(buf, strtok(NULL, " \n"));
        base = buf;
        rqst_time = strtol(buf, &base, 10);
        if (*base != '\0' || (rqst_time <= 0 && rqst_time > 60)) {
            printf("%s invalid wholast %s(%ld)\n", name, buf, rqst_time);
            len = sprintf(buf, "SERVER: invalid wholast input,"
                    "must be between 0 and 60\n");
            send(sock, buf, len, 0);
        } else {
            sprintf(buf, "Current Users<Login in last %ld minutes>:\n", rqst_time);
            node_itr = usr_lst->head;
            while (node_itr) {
                User_Sock *wholast_usr = (User_Sock *) (node_itr->data);
                if (usr_sock->name &&
                    time(NULL) - wholast_usr->login < rqst_time * 60) {
                    len = sprintf(buf, "\t%s\n", wholast_usr->name);
                    send(sock, buf, len, 0);
                }
                node_itr = node_itr->next;
            }
        }
    }
    return;

    end:
    len = sprintf(buf, "Bad Command Syntax\n");
    send(sock, buf, len, 0);
}