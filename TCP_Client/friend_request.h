#ifndef CLIENT_FRIEND_REQUEST_H
#define CLIENT_FRIEND_REQUEST_H

#include "common.h"

#define REQ_SEND_FRIEND_REQUEST "SEND_FRIEND_REQUEST"
#define REQ_GET_FRIEND_REQUESTS "GET_FRIEND_REQUESTS"

// Function declarations
void do_send_friend_request(int sock);
void do_get_friend_requests(int sock);

#endif // CLIENT_FRIEND_REQUEST_H

