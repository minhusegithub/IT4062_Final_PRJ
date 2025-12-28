#ifndef CLIENT_FRIEND_H
#define CLIENT_FRIEND_H

#include "common.h"

#define REQ_GET_FRIENDS "GET_FRIENDS"
#define REQ_UNFRIEND "UNFRIEND"

void do_get_friends_list(int sock);
void do_unfriend(int sock);

#endif // CLIENT_FRIEND_H