//
// Created by Theodore Ahlfeld on 9/20/15.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "auth.h"

extern int MAXBUF;

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

Credential *load_credentials(const char *file_loc,
                             Credential *cred_list, const size_t cred_nel) {
    FILE *fp_cred = fopen(file_loc, "rb");
    char user[MAXBUF], pass[MAXBUF];
    cred_list = (Credential *) malloc(sizeof(Credential) * cred_nel);
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

int count_lines(const char *file_loc) {
    FILE *fp_cred = fopen(file_loc, "rb");
    if (fp_cred == NULL) {
        die_with_err("Could not open file\nError Code: ");
    }
    int c = 0;
    int lines = 0;
    while (c != EOF) {
        if ((c = fgetc(fp_cred)) == '\n') lines++;
    }

    fclose(fp_cred);
    return lines;
}

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

void unban_usr(List *ban_list, Node *usr_node) {
    Banned_User *usr = remove_node(ban_list, usr_node);
    free(usr->ip);
    free(usr->rsn);
    free(usr);
    usr_node = NULL;
}

Banned_User * check_ban(List *ban_list, const char *ip) {
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

void remove_all_credentials(Credential *lst, long nel) {
    long i;
    for (i = 0; i < nel && &(lst[i]) != NULL; i++) {
        free((&lst[i])->pass);
        free((&lst[i])->user);
    }
    free(lst);
    lst = NULL;
}