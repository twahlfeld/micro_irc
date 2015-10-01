//
// Created by Theodore Ahlfeld on 9/22/15.
//

#ifndef __SRV_IO_H
#define __SRV_IO_H

#include "list.h"
#include "server.h"

void broadcast(const List *usr_lst, const char *msg, int self_sock);
void private_msg(char *message, const User_Sock *src);
void client_cmd(List *usr_lst, Node *usr_node, char *msg);

#endif
