

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "auth.h"

extern int MAXBUF;

/*
 * Finds a username in an array of Credentials through binary search
 * const char *user -> The username to look for
 * const Credential *cred_list  -> The array of Credentials
 * const size_t cred_nel    -> The number of elements
 * returns the Credential if found otherwise NULL
 */
Credential *find_cred(const char *user,
                      const Credential *cred_list, const size_t cred_nel) {
    if (user) {
        long l_bound = 0, r_bound = cred_nel - 1;
        while (l_bound <= r_bound) { ;
            long middle = (l_bound + r_bound) / 2;
            Credential cred = cred_list[middle];
            int compare = strcasecmp(user, cred.user);
            if (compare == 0) {
                return (Credential *) (&cred_list[middle]);
            } else if (compare < 0) {
                r_bound = middle - 1;
            } else if (compare > 0) {
                l_bound = middle + 1;
            }
        }
    }
    return NULL;
}

/*
 * Load an array of credentials from files
 * INCLUDES SIDE-EFFECTS
 * const char *file_loc -> file name location
 * Credential *cred_list-> A Credential pointer to write the list into
 * size_t *cred_nel -> the number of elements (SIDE-EFFECT if 0)
 * returns the array of loaded credential form file file_loc
 */
Credential *load_credentials(const char *file_loc,
                             Credential *cred_list, size_t *cred_nel) {
    if(*cred_nel == 0) {
        *cred_nel = count_lines(file_loc);
    }
    FILE *fp_cred = fopen(file_loc, "rb");
    char user[MAXBUF], pass[MAXBUF];
    cred_list = (Credential *) malloc(sizeof(Credential) * *cred_nel);
    char buf[MAXBUF];
    int i;
    for (i = 0; fgets(buf, sizeof(buf), fp_cred); i++) {
        strcpy(user, strtok(buf, " "));
        strcpy(pass, strtok(NULL, "\n"));
        (cred_list[i]).user = (char *) malloc(strlen(user) + 1);
        (cred_list[i]).pass = (char *) malloc(strlen(pass) + 1);
        strcpy((cred_list[i]).user, user);
        strcpy((cred_list[i]).pass, pass);
        cred_list[i].in_use = 0;
    }
    fclose(fp_cred);
    return cred_list;
}

/*
 * Counts the lines of the file
 * Used to find how much allocation for cred_list is needed
 * const char *file_loc -> File name to upon
 * return the amount of newlines(credentials) in the file
 */
size_t count_lines(const char *file_loc) {
    FILE *fp_cred = fopen(file_loc, "rb");
    if (fp_cred == NULL) {
        die_with_err("Could not open file\nError Code: ");
    }
    int c = 0;
    size_t lines = 0;
    while (c != EOF) {
        if ((c = fgetc(fp_cred)) == '\n') lines++;
    }

    fclose(fp_cred);
    return lines;
}

/*
 * Bans the user
 * List *ban_lst -> A linked list of banned users
 * const char *ip -> The banned users ip
 * const char *reason -> The reason for ban()
 */
void ban_usr(List *ban_lst, const char *ip,
             const char *reason, const time_t ban_time)
{
    Banned_User *usr = (Banned_User *)malloc(sizeof(Banned_User));
    usr->ip = (char *)malloc(sizeof("XXX.XXX.XXX.XXX0"));
    usr->rsn = (char *)malloc(strlen(reason)+1);
    strcpy(usr->ip, ip);
    strcpy(usr->rsn, reason);
    usr->ban_expire = (time(NULL) + (ban_time * 60));
    printf("%s banned for %ld minutes for %s", ip, ban_time, reason);
    add_end(ban_lst, usr);
}

/*
 * Unbans a client's IP
 * List *ban_list   -> List of banned users
 * Node *usr_node   -> The node to remove from the banned list
 */
void unban_usr(List *ban_list, Node *usr_node) {
    Banned_User *usr = remove_node(ban_list, usr_node);
    free(usr->ip);
    free(usr->rsn);
    free(usr);
    usr_node = NULL;
}

/*
 * Checks the ban time by comparing current-time vs ban expired
 * List *ban_list   -> The list of banned users
 * const char *ip   -> The IP to check for ban
 * returns the Banned User if found otherwise NULL
 */
Banned_User *check_ban(List *ban_list, const char *ip) {
    Node *usr_node = ban_list->head;
    time_t cur_time = time(NULL);
    Banned_User *ban_usr = NULL;
    while(usr_node) {
        Banned_User *usr = (Banned_User *)(usr_node->data);
        if(usr->ban_expire < cur_time) {
            unban_usr(ban_list, usr_node);
        } else {
            ban_usr = ((strcmp(ip, usr->ip) == 0) ? usr : NULL);
        }
        if(usr_node) {
            usr_node = usr_node->next;
        }
    }
    return ban_usr;
}

/*
 * Removes all credentials from an array of credentials
 * Credential *lst  -> The array of Credentials
 * size_t nel       -> number of elements for the array of Credentials
 */
void remove_all_credentials(Credential *lst, size_t nel) {
    unsigned long i;
    for (i = 0; i < nel && &(lst[i]) != NULL; i++) {
        free((&lst[i])->pass);
        free((&lst[i])->user);
    }
    free(lst);
    lst = NULL;
}