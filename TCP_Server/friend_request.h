#ifndef FRIEND_REQUEST_H
#define FRIEND_REQUEST_H

#include "common.h"
#include "account.h"

#define FRIEND_REQUEST_FILE_PATH "TCP_Server/data/friend_request.txt"
#define MAX_FRIEND_REQUESTS 1024  // Up to 1024 different users can have an invitation list.
#define MAX_FRIEND_REQUESTS_PER_USER 500  // A user can receive a maximum of 500 invitations.

// Request command
#define REQ_SEND_FRIEND_REQUEST "SEND_FRIEND_REQUEST"
#define REQ_GET_FRIEND_REQUESTS "GET_FRIEND_REQUESTS"
#define REQ_ACCEPT_FRIEND_REQUEST "ACCEPT_FRIEND_REQUEST"
#define REQ_REJECT_FRIEND_REQUEST "REJECT_FRIEND_REQUEST"

// Response messages
#define MSG_FRIEND_REQUEST_SENT_SUCCESS "Friend request sent successfully"
#define MSG_USER_NOT_FOUND "User not found"
#define MSG_ALREADY_SENT "Friend request already sent"
#define MSG_SELF_REQUEST "Cannot send friend request to yourself"
#define MSG_TOO_MANY_REQUEST_PER_USER "User has too many requests"
#define MSG_NO_FRIEND_REQUEST_LIST "The current user could not be found in the corresponding database"
#define MSG_NOT_FOUND_FRIEND_REQUEST "Friend request not found"
#define MSG_NO_INVITATION "The user has no invitation"
#define MSG_GET_FRIEND_REQUESTS_SUCCESS "Get friend request list successfully"
#define MSG_REMOVE_REQ_FAILED "Failed to remove request"
#define MSG_FRIEND_REQUEST_ACCEPT_SUCCESS "Friend request accepted successfully"
#define MSG_FRIEND_REQUEST_REJECT_SUCCESS "Friend request rejected successfully"


// Structure to store friend requests for a user
typedef struct {
    int user_id;
    int request_count;
    int request_ids[MAX_FRIEND_REQUESTS_PER_USER];  // List of people who sent invitations to user_id
} FriendRequestList;

// Global variables
extern FriendRequestList friend_requests[MAX_FRIEND_REQUESTS];
extern int friend_request_count;

// Function declarations
int load_friend_requests(const char *filename);
int save_friend_requests(const char *filename);
FriendRequestList* find_friend_request_list(int user_id);
int has_sent_request(int from_user_id, int to_user_id);
void handle_send_friend_request(int client_index, char *args);
void handle_get_friend_requests(int client_index);
void handle_accept_friend_request(int client_index, char *args);
void handle_reject_friend_request(int client_index, char *args);
int remove_friend_request_from_list(int user_id, int from_user_id);

#endif // FRIEND_REQUEST_H

