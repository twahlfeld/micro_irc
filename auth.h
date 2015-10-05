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

/*
 * Finds a username in an array of Credentials through binary search
 * const char *user -> The username to look for
 * const Credential *cred_list  -> The array of Credentials
 * const size_t cred_nel    -> The number of elements
 * return the Credential if found otherwise NULL
 */
Credential *find_cred(const char *user,
                      const Credential *cred_list, const size_t cred_nel);


/*
 * Load an array of credentials from files
 * INCLUDES SIDE-EFFECTS
 * const char *file_loc -> file name location
 * Credential *cred_list-> A Credential pointer to write the list into
 * size_t *cred_nel -> the number of elements (SIDE-EFFECT if 0)
 * returns the array of loaded credential form file file_loc
 */
Credential *load_credentials(const char *file_loc,
                             Credential *cred_list, size_t *cred_nel);

/*
 * Counts the lines of the file
 * Used to find how much allocation for cred_list is needed
 * const char *file_loc -> File name to upon
 * return the amount of newlines(credentials) in the file
 */
size_t count_lines(const char *file_loc);

/*
 * Removes all credentials from an array of credentials
 * Credential *lst  -> The array of Credentials
 * size_t nel       -> number of elements for the array of Credentials
 */
void remove_all_credentials(Credential *lst, size_t nel);

/*
 * Bans the user
 * List *ban_lst -> A linked list of banned users
 * const char *ip -> The banned users ip
 * const char *reason -> The reason for ban()
 */
void ban_usr(List *ban_lst, const char *ip,
             const char *reason, const time_t ban_time);

/*
 * Checks the ban time by comparing current-time vs ban expired
 * List *ban_list   -> The list of banned users
 * const char *ip   -> The IP to check for ban
 * returns the Banned User if found otherwise NULL
 */
Banned_User * check_ban(List *ban_list, const char *ip);

#endif
