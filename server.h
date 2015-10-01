/*
 * =====================================================================================
 *
 *       Filename:  server.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  09/19/2015 19:27:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Theodore Ahlfeld (), twahlfeld@gmail.com
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef __IRC_SERVER_H
#define __IRC_SERVER_H

#include <time.h>

typedef struct User_Sock {
    int sock;
    char *name;
    time_t login;
    char *ip;
} User_Sock;

User_Sock *find_usr(const char *usr);
void set_motd(char *msg, char *usr);
void print_motd(int sock);
void kill_user(void *us, int flag);
void die_with_err(const char *err_msg);
int cmpusr(const void *a, const void *b);

#endif
