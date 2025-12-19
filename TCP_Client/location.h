#ifndef CLIENT_LOCATION_H
#define CLIENT_LOCATION_H

// Protocol commands
#define REQ_ADD_LOCATION    "ADD_LOCATION"
#define REQ_GET_LOCATIONS   "GET_LOCATIONS"
#define REQ_UPDATE_LOCATION "UPDATE_LOCATION"
#define REQ_DELETE_LOCATION "DELETE_LOCATION"
#define REQ_VIEW_MY_LOCATIONS "VIEW_MY_LOCATIONS"
#define MSG_END_DATA        "End of data"

void do_add_location(int sock);
void do_get_locations(int sock);
void do_update_location(int sock);
void do_delete_location(int sock);
void do_view_my_locations(int sock);

#endif // CLIENT_LOCATION_H