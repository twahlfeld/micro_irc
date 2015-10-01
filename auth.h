//
// Created by Theodore Ahlfeld on 9/20/15.
//

#ifndef __AUTH_C_H
#define __AUTH_C_H

#include <time.h>
#include "server.h"
#include "list.h"

typedef struct Credential {
    char *user;
    char *pass;
    int in_use;
    time_t suspend;
} Credential;

typedef struct Banned_User {
    char *ip;
    char *rsn;
    time_t ban_expire;
} Banned_User;

Credential *find_cred(const char *user,
                      const Credential *cred_list, const size_t cred_nel);
Credential *load_credentials(const char *file_loc,
                             Credential *cred_list, const size_t cred_nel);
int count_lines(const char *file_loc);
void remove_all_credentials(Credential *lst, long nel);
void ban_usr(List *ban_lst, const char *ip,
             const char *reason, const time_t ban_time);
Banned_User * check_ban(List *ban_list, const char *ip);

#endif
