#ifndef FRIEND_REQUEST_H
#define FRIEND_REQUEST_H

#include "common.h"
#include "account.h"

#define FRIEND_REQUEST_FILE_PATH "TCP_Server/data/friend_request.txt"
#define MAX_FRIEND_REQUESTS 1024  // Tối đa 1024 users khác nhau có thể có danh sách lời mời.
#define MAX_FRIEND_REQUESTS_PER_USER 500  // Tối đa 500 lời mời mà một user có thể nhận

// Request command
#define REQ_SEND_FRIEND_REQUEST "SEND_FRIEND_REQUEST"

// Response messages
#define MSG_FRIEND_REQUEST_SUCCESS "Friend request sent successfully"
#define MSG_USER_NOT_FOUND "User not found"
#define MSG_ALREADY_SENT "Friend request already sent"
#define MSG_SELF_REQUEST "Cannot send friend request to yourself"

// Structure to store friend requests for a user
typedef struct {
    int user_id;
    int request_count;
    int request_ids[MAX_FRIEND_REQUESTS_PER_USER];  // Danh sách user_id đã gửi lời mời
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

#endif // FRIEND_REQUEST_H

