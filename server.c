/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  09/19/2015 14:53:11
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Theodore Ahlfeld (), twahlfeld@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */

#define DEBUG


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "list.h"
#include "server.h"
#include "auth.h"
#include "srv_io.h"

#define AUTH_FILE_LOCATION "./user_pass.txt"
#define MAX_CONNECTIONS 100
#define MAX_PASS_ATTEMPTS 3
#define BLOCK_TIME 60
#define DEFAULT_MOTD "Welcome to simple chat server!"
#define TIMEOUT_FLAG 1
#define BANNED_FLAG 2
#define TIME_OUT 30*60

fd_set mstr_fds, rdonly_fds;;
const size_t MAXBUF = 256;
static Credential *cred_list;
static size_t cred_nel;
char motd[128];
int serv_sock, timeout = TIME_OUT;
List usr_lst, ban_lst;

void die_with_err(const char *err_msg)
{
    perror(err_msg);
    exit(1);
}

int cmpusr(const void *a, const void *b)
{
    char *name = ((User_Sock *) b)->name;
    if (name) {
        return strcasecmp((char *)a, name);
    }
    return -1;
}

User_Sock *create_usr_sock(int sock, struct sockaddr_in *addr)
{
    User_Sock *usr_sock = (User_Sock *) malloc(sizeof(User_Sock));
    usr_sock->sock = sock;
    usr_sock->ip = (char *) malloc(strlen("XXX.XXX.XXX.XXX0"));
    strcpy(usr_sock->ip, inet_ntoa(addr->sin_addr));
    FD_SET(sock, &mstr_fds);
    usr_sock->name = NULL;
    usr_sock->login = time(NULL);
    return usr_sock;
}

void kill_user(void *us, int flag)
{
    User_Sock *usr_sock = (User_Sock *) us;
    Credential *cred = find_cred(usr_sock->name, cred_list, cred_nel);
    int sock = usr_sock->sock;
    char buf[MAXBUF], reason[10];
    if (flag == TIMEOUT_FLAG) {
        strcpy(reason, "timeout");
    } else if (flag == BANNED_FLAG) {
        strcpy(reason, "banned");
        ban_usr(&ban_lst, usr_sock->ip, reason, time(NULL));
    } else {
        strcpy(reason, "quit");
    }
    printf("%s(socket %d) has disconnected(%s)\n", usr_sock->ip, usr_sock->sock, reason);
    if (cred) {
        sprintf(buf, "%s has disconnected(%s)\n", cred->user, reason);
        broadcast(&usr_lst, buf, sock);
        cred->in_use = 0;
    }
    FD_CLR(sock, &rdonly_fds);
    FD_CLR(sock, &mstr_fds);
    shutdown(sock, SHUT_RDWR);
    close(sock);
    free(usr_sock->ip);
    free(usr_sock);
    usr_sock = NULL;
}

void dft_kill(void *us)
{
    kill_user(us, 0);
}

static void die_gracefully(int sigint)
{
    traverse_list(&usr_lst, dft_kill);
    remove_all_nodes(&usr_lst);
    remove_all_credentials(cred_list, cred_nel);
    shutdown(serv_sock, SHUT_RDWR);
    close(serv_sock);
    printf("Server Ended with code %d\n", sigint);
    exit(0);
}

int cmp(const void *a, const void *b)
{
    return strcasecmp(((Credential *) a)->user, ((Credential *) b)->user);
}

struct sockaddr_in init_socket_addr(const long long addr, unsigned short port)
{
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(addr);
    sock_addr.sin_port = htons(port);
    return sock_addr;
}

void close_sock(const char *srv_msg, const char *broad_msg, User_Sock *us)
{
    if (srv_msg) {
        printf("%s", srv_msg);
    }
    broadcast(&usr_lst, broad_msg, us->sock);
    kill_user(us, 0);
}

void set_motd(char *msg, char *usr)
{
    if (usr) {
        printf("%s changed motd to %s\n", usr, msg);
    }
    strncpy(motd, msg, sizeof(motd));
}

void print_motd(int sock)
{
    char buf[MAXBUF];
    size_t len;
    len = sprintf(buf, "%s\n", motd);
    send(sock, buf, len, 0);
}

void *connect_client(User_Sock *usr_sock)
{
    int sock = usr_sock->sock;
    usr_sock->name = NULL;;
    Credential *cred;
    char buf[MAXBUF];
    ssize_t len;
    char *srv_msg = NULL, *brd_msg = NULL;
    len = sprintf(buf, "Username: ");
    buf[sizeof(buf)-1] = '\0';
    send(sock, buf, len, 0);
    if ((len = recv(sock, buf, sizeof(buf), 0)) <= 0) {
        if (len < 0) {
            perror("recv() failed");
            goto end;
        }
    }
    buf[--len] = '\0';

    if ((cred = find_cred(buf, cred_list, cred_nel)) == 0) {
        strcpy(buf, "Username was not found\n");
        send(sock, buf, strlen(buf), 0);
        goto end;
    }
    usr_sock->name = cred->user;
    if (cred->in_use) {
        len = sprintf(buf, "User %s is already logged in\n", usr_sock->name);
        send(sock, buf, len, 0);
        goto end;
    } else if (time(NULL) - cred->suspend <= BLOCK_TIME) {

    }
    int i;
    for (i = 0; i < MAX_PASS_ATTEMPTS; i++) {
        strcpy(buf, "Password: ");
        send(sock, buf, strlen(buf), 0);
        if ((len = recv(sock, buf, strlen(buf), 0)) <= 0) {
            if (len <= 0) {
                perror("recv() failed");
                goto end;
            }
        }
        buf[--len] = '\0';
        if (strncmp(buf, cred->pass, (size_t) len) == 0) {
            cred->in_use = 1;
            sprintf(buf, "User %s has connected\n", usr_sock->name);
            printf("%s logged in as %s\n", usr_sock->ip, usr_sock->name);
            broadcast(&usr_lst, buf, sock);
            print_motd(sock);
            add_front(&usr_lst, usr_sock);
            return NULL;
        } else {
            len = sprintf(buf, "Wrong Password\n");
            if ((send(sock, buf, len, 0)) < 0) {
                perror("send() failed");
                goto end;
            }
        }
    }
    len = sprintf(buf, "Maximum Password Attempts have been exceeded\n");
    send(sock, buf, len, 0);
    ban_usr(&ban_lst, usr_sock->ip, buf, BLOCK_TIME);
    srv_msg = buf;

    end:
    close_sock(srv_msg, brd_msg, usr_sock);
    return NULL;
}

User_Sock *find_usr(const char *usr)
{
    if (usr) {
        Node *usr_node = find_node(&usr_lst, usr, cmpusr);
        if (usr_node) {
            return (User_Sock *) (usr_node->data);
        }
    }
    return NULL;
}

int accept_connection(struct sockaddr_in *clnt_addr, int fdmax)
{
    pthread_t thread;
    User_Sock *usr_sock;
    int sock;
    size_t len;
    char buf[MAXBUF];
    socklen_t socklen = sizeof(struct sockaddr_in);
    if ((sock = accept(serv_sock, (struct sockaddr *) clnt_addr, &socklen)) == -1) {
        perror("accept() failed");
    } else {
        Banned_User *usr = NULL;
        if ((usr = check_ban(&ban_lst, inet_ntoa(clnt_addr->sin_addr)))) {
            len = sprintf(buf, "Banned for %s for %ld minutes\nreason: %s\n",
                         usr->ip, (usr->ban_expire - time(NULL))/60, usr->rsn);
            send(sock, buf, len, 0);
            shutdown(sock, SHUT_RDWR);
            close(sock);
            FD_CLR(sock, &mstr_fds);
        } else {
            fdmax = (fdmax > sock ? fdmax : sock);
            usr_sock = create_usr_sock(sock, clnt_addr);
            printf("%s attempting to connect\n", usr_sock->ip);
            pthread_create(&(thread), NULL,
                           (void *) connect_client, usr_sock);
        }
    }
    return fdmax;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        die_with_err("No port argument found");
    }
    /*
     * Setting up file credentials for IRC server
     */
    char *file_loc = AUTH_FILE_LOCATION;
    cred_nel = (size_t) count_lines(file_loc);
    cred_list = load_credentials(file_loc, cred_list, cred_nel);
    qsort(cred_list, cred_nel, sizeof(Credential), cmp);

    char buf[MAXBUF];
    /*
     * Setting up server socket data
     */
    //fd_set rdonly_fds;    /* master and read_only file descriptors*/
    struct sockaddr_in serv_addr; /* Local address */
    struct sockaddr_in clnt_addr; /* Client address */
    unsigned short port = (unsigned short) atoi(argv[1]);
    serv_addr = init_socket_addr(INADDR_ANY, port);
    int enabled = 1, fdmax;
    struct timeval block_timev;
    ssize_t len;
    set_motd(DEFAULT_MOTD, NULL);

    /*
     * Setting up server connection
     */
    if ((serv_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        die_with_err("socket() failed");
    }
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(int));
    setsockopt(serv_sock, SOL_SOCKET, SO_KEEPALIVE, &enabled, sizeof(int));
    fcntl(serv_sock, F_SETFL, O_NONBLOCK);
    if (bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        die_with_err("bind() failed");
    }
    if (listen(serv_sock, MAX_CONNECTIONS) < 0) {
        die_with_err("listen() failed");
    }
    memset(&clnt_addr, 0, sizeof(clnt_addr));

    FD_ZERO(&mstr_fds);
    User_Sock *usr_sock = create_usr_sock(serv_sock, &serv_addr);
    init_list(&usr_lst);
    init_list(&ban_lst);
    add_front(&usr_lst, usr_sock);
    fdmax = serv_sock;
    for (; ;) { // Run til signal interrupt
        rdonly_fds = mstr_fds;
        signal(SIGINT, die_gracefully);
        signal(SIGTERM, die_gracefully);
        block_timev.tv_sec = timeout;
        block_timev.tv_usec = 0;
        if (select(fdmax + 1, &rdonly_fds, NULL, NULL,
                   (timeout ? &block_timev : NULL)) < 0) {
            //die_with_err("select() failed");
            perror("select() failed");
        }

        Node *usr_node = usr_lst.head;

        while (usr_node) {
            usr_sock = (User_Sock *) (usr_node->data);
            if (usr_sock != NULL) {
                int sock = usr_sock->sock;
                if (FD_ISSET(sock, &rdonly_fds)) {
                    if (sock == serv_sock) {
                        fdmax = accept_connection(&clnt_addr, fdmax);
                    } else {
                        if ((len = recv(sock, buf, sizeof(buf), 0)) <= 0) {
                            // got error or connection closed by client
                            if (len == 0) {
                                // connection closed
                                printf("Connection Closed\n");
                            } else {
                                perror("recv() failed");
                            }
                            remove_node(&usr_lst, usr_node);
                            kill_user(usr_sock, TIMEOUT_FLAG);
                            continue;
                        } else {
                            client_cmd(&usr_lst, usr_node, buf);
                        }
                    }
                }
            }
            usr_node = usr_node->next;
        }
    }
}