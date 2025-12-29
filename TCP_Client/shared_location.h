#ifndef CLIENT_SHARED_H
#define CLIENT_SHARED_H
#include "common.h"

#define REQ_SHARE_LOCATION "SHARE_LOCATION"
#define REQ_GET_SHARED_LOCATIONS "GET_SHARED_LOCATIONS"

void do_share_location(int sock);
void do_get_shared_locations(int sock);

#endif