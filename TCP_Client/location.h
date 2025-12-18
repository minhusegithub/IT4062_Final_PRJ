#ifndef CLIENT_LOCATION_H
#define CLIENT_LOCATION_H

// Protocol commands
#define REQ_ADD_LOCATION    "ADD_LOCATION"
#define REQ_GET_LOCATIONS   "GET_LOCATIONS"
#define MSG_END_DATA        "End of data"

void do_add_location(int sock);
void do_get_locations(int sock);

#endif // CLIENT_LOCATION_H