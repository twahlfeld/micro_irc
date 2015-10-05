/*
 * ============================================================================
 *
 *       Filename:  server.h
 *
 *    Description:  A simple IRC program
 *
 *      Functions:
 *          void set_motd(char *msg, char *usr) -> Changes the MOTD
 *          void print_motd(int sock)           -> Prints the MOTD
 *          void kill_user(void *us, int flag)  -> Removes the user
 *          void die_with_err(const char *err_msg) -> Kills server with error
 *          int cmpusr(const void *a, const void *b) ->
 *              Compares users name between a string and a User_Sock struct
 *
 *        Version:  1.0
 *        Created:  09/19/2015 19:27:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Theodore Ahlfeld (), twahlfeld@gmail.com
 *   Organization:
 *
 * ============================================================================
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

/*
 * Finds a current user
 * const char *usr  -> Username to find
 * returns the User_Sock if found otherwise NULL
 */
User_Sock *find_usr(const char *usr);

/*
 * Changes the motd or welcome message
 * char *msg    -> The new motd
 * char *usr    -> The name of the user that changed it
 */
void set_motd(char *msg, char *usr);

/*
 * Prints the motd to a specified user
 * int sock -> The socket to print the motd to
 */
void print_motd(int sock);

/*
 * Disconnects a user and cleans up all resources
 * void *us -> The User_Socket to remove
 * int flag -> The flag for reason of leaving default is quit
 */
void kill_user(void *us, int flag);

/*
 * Error checking for critical procedures
 * const char *err_msg  -> Function that caused the failure
 */
void die_with_err(const char *err_msg);

/*
 * Compares users name between a string and a User_Sock struct
 * const void *a    -> string of the user name to find
 * const void *b    -> The User_Sock struct to test for
 * returns the difference between the string and the User name.
 */
int cmpusr(const void *a, const void *b);

#endif
