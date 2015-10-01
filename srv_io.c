//
// Created by Theodore Ahlfeld on 9/22/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include "server.h"
#include "srv_io.h"

extern size_t MAXBUF;
extern fd_set mstr_fds;
extern int serv_sock;
extern int timeout;

void broadcast(const List *usr_lst, const char *msg, int self_sock) {
    if (msg) {
        printf("%s", msg);
        Node *clnt_node = usr_lst->head;
        while (clnt_node) {
            User_Sock *usr_sock = (User_Sock *) (clnt_node->data);
            if (usr_sock) {
                int tmp_sock = usr_sock->sock;

                // send to everyone!
                if (FD_ISSET(tmp_sock, &mstr_fds)) {
                    // except the listener and ourselves
                    if (tmp_sock != serv_sock && tmp_sock != self_sock) {
                        if (send(tmp_sock, msg, (size_t) strlen(msg), 0) == -1) {
                            perror("send() failed");
                        }
                    }
                }
            }
            clnt_node = clnt_node->next;
        }
    }
}

void pvt_broadcast(char *rol, int sock) {
    List pvt_usr_list;
    init_list(&pvt_usr_list);
    if (rol) {
        User_Sock *usr_sock;
        char *buf = strtok(rol, " \n");
        char msg[MAXBUF];
        while (buf) {
            if (strcasecmp(buf, "message") == 0) {
                buf = strtok(NULL, "\n");
                if (buf == NULL) goto cleanup;
                sprintf(msg, "%s\n", buf);
                broadcast(&pvt_usr_list, msg, sock);
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

void private_msg(char *message, const User_Sock *src) {
    char buf[MAXBUF];
    char *base;

    if (message == NULL) goto end;
    if ((base = strtok(message, " \n")) == NULL) goto end;

    User_Sock *dst = find_usr(base);
    if (dst == NULL && (dst->name) == NULL) goto end;
    if ((base = strtok(NULL, "\n")) == NULL) goto end;
    fprintf(stderr, "%s->", src->name);
    fprintf(stderr, "%s:", dst->name);
    fprintf(stderr, "%s\n", base);
    printf("%s->%s: %s\n", src->name, dst->name, base);
    sprintf(buf, "%s: %s\n", src->name, base);
    send(dst->sock, buf, strlen(buf), 0);

    end:
    return;
}

void client_cmd(List *usr_lst, Node *usr_node, char *msg) {
    User_Sock *usr_sock = (User_Sock *) (usr_node->data);
    char *name = usr_sock->name;
    int sock = usr_sock->sock;
    char *cmd = strtok(msg, " \n"), *base;
    if(cmd == NULL) return;
    Node *node_itr = usr_lst->head;
    char buf[MAXBUF];
    if (strcasecmp(cmd, "quit") == 0 || strcasecmp(cmd, "logout") == 0) {
        remove_node(usr_lst, usr_node);
        kill_user(usr_sock, 0);
    } else if (strcasecmp(cmd, "broadcast") == 0) {

        if ((base = strtok(NULL, " \n")) == NULL) {
            goto end;
        } else {
            if (strcasecmp(base, "message") == 0) {
                sprintf(buf, "%s: %s\n", name, strtok(NULL, "\n"));
                broadcast(usr_lst, buf, sock);
            } else if (strcasecmp(base, "user") == 0) {
                pvt_broadcast(strtok(NULL, "\n"), sock);
            }
        }
    } else if (strcasecmp(cmd, "message") == 0) {
        private_msg(strtok(NULL, "\n"), usr_sock);
    } else if (strcasecmp(cmd, "motd") == 0) {
        strcpy(buf, strtok(NULL, "\n"));
        if (strlen(buf)) {
            set_motd(buf, name);
        } else {
            print_motd(sock);
        }
    } else if (strcasecmp(cmd, "timeout") == 0) {
        int new_timeout;
        strcpy(buf, strtok(NULL, " \n"));
        char *timeout_base = buf;
        new_timeout = (int) strtol(buf, &timeout_base, 10);
        if (*timeout_base != '\0') {
            printf("%s failed to change timeout to %s\n", name, timeout_base);
            strcpy(buf, "SERVER: invalid input timeout\n");
            send(sock, buf, strlen(buf), 0);
        } else {
            timeout = new_timeout;
            sprintf(buf, "%s changed timeout %d\n", name, timeout);
            broadcast(usr_lst, buf, -1);
        }
    } else if (strcasecmp(cmd, "whoelse") == 0) {
        char *active_client;
        sprintf(buf, "Current Users:\n");
        send(sock, buf, strlen(buf), 0);
        while (node_itr) {
            if ((active_client = ((User_Sock *) (node_itr->data))->name)) {
                sprintf(buf, "\t%s\n", active_client);
                send(sock, buf, strlen(buf), 0);
            }
            node_itr = node_itr->next;
        }
    } else if (strcasecmp(cmd, "wholast") == 0) {
        time_t rqst_time;
        strcpy(buf, strtok(NULL, " \n"));
        char *base = buf;
        rqst_time = strtol(buf, &base, 10);
        if (*base != '\0' || (rqst_time <= 0 && rqst_time > 60)) {
            printf("%s invalid wholast %s(%ld)\n", name, buf, rqst_time);
            strcpy(buf, "SERVER: invalid wholast input, must be between 0 and 60\n");
            send(sock, buf, strlen(buf), 0);
        } else {
            sprintf(buf, "Current Users<Login in last %ld minutes>:\n", rqst_time);
            node_itr = usr_lst->head;
            while (node_itr) {
                User_Sock *wholast_usr = (User_Sock *) (node_itr->data);
                if (usr_sock->name &&
                    time(NULL) - wholast_usr->login < rqst_time * 60) {
                    sprintf(buf, "\t%s\n", wholast_usr->name);
                    send(sock, buf, strlen(buf), 0);
                }
                node_itr = node_itr->next;
            }
        }
    }
    return;

    end:
    strcpy(buf, "Bad Command Syntax");
    send(sock, buf, strlen(buf), 0);
}