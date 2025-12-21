#ifndef CLIENT_FRIEND_REQUEST_H
#define CLIENT_FRIEND_REQUEST_H

#include "common.h"

#define REQ_SEND_FRIEND_REQUEST "SEND_FRIEND_REQUEST"
#define REQ_GET_FRIEND_REQUESTS "GET_FRIEND_REQUESTS"
#define REQ_ACCEPT_FRIEND_REQUEST "ACCEPT_FRIEND_REQUEST"
#define REQ_REJECT_FRIEND_REQUEST "REJECT_FRIEND_REQUEST"

// Function declarations
void do_send_friend_request(int sock);
void do_get_friend_requests(int sock);
void do_accept_friend_request(int sock);
void do_reject_friend_request(int sock);

#endif // CLIENT_FRIEND_REQUEST_H

