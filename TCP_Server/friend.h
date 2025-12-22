#ifndef FRIEND_H
#define FRIEND_H

#include "common.h"
#include "account.h"

#define FRIEND_FILE_PATH "TCP_Server/data/friend.txt"
#define MAX_FRIENDS 1024  // Up to 1024 different users can have a friend list
#define MAX_FRIENDS_PER_USER 500  // A user can have a maximum of 500 friends
#define MSG_ALREADY_FRIENDS "Already friends"
#define MSG_TOO_MANY_FRIENDS "Too many friends"

// Request command
#define REQ_GET_FRIENDS "GET_FRIENDS"

// Response messages
#define MSG_GET_FRIENDS_SUCCESS "Get friends list successfully"
#define MSG_NO_FRIENDS "You have no friends"

// Structure to store friends for a user
typedef struct {
    int user_id;
    int friend_count;
    int friend_ids[MAX_FRIENDS_PER_USER];  // List of friend user_ids
} FriendList;

// Global variables
extern FriendList friends[MAX_FRIENDS];
extern int friend_count;

// Function declarations
int load_friends(const char *filename);
int save_friends(const char *filename);
FriendList* find_friend_list(int user_id);
int add_friend(Client *client, int user_id, int friend_id);
void handle_get_friends(int client_index);

#endif // FRIEND_H

