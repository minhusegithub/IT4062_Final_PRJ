#ifndef SHARED_LOCATION_H
#define SHARED_LOCATION_H

#include "common.h"
#include "account.h"
#include "location.h"
#include "friend.h"

#define SHARED_FILE_PATH "TCP_Server/data/taged_location.txt"
#define MAX_SHARED_PER_USER 100
#define MAX_SHARED_LISTS 1024

// Protocol Commands
#define REQ_SHARE_LOCATION "SHARE_LOCATION"
#define REQ_GET_SHARED_LOCATIONS "GET_SHARED_LOCATIONS"

// Response Messages
#define MSG_SHARE_SUCCESS "Share location successful"
#define MSG_SHARED_READ_SUCCESS "Get shared locations successfully"
#define MSG_NOT_FRIEND_SHARE "Username is not your friend"
#define MSG_NO_SHARED_LOC "You have no shared locations"
#define MSG_LOC_NAME_NOT_FOUND "Location name not found"
#define MSG_SHARED_LIST_FULL "Shared list is full"

/**
 * Structure to represent a shared item
 */
typedef struct {
    int sharer_id;   // ID người chia sẻ (người gửi)
    int location_id; // ID địa điểm
} SharedItem;

/**
 * Structure to store shared locations for a user (The receiver)
 * Format file: user_id|n|sharer_id1 loc_id1 sharer_id2 loc_id2 ...
 */
typedef struct {
    int user_id; 
    int count;
    SharedItem items[MAX_SHARED_PER_USER];
} SharedList;

// Global variables
extern SharedList shared_lists[MAX_SHARED_LISTS];
extern int shared_count;

// Functions
int load_shared_locations(const char *filename);
int save_shared_locations(const char *filename);
void handle_share_location(int client_index, char *args);
void handle_get_shared_locations(int client_index);
SharedList* find_shared_list(int user_id);

#endif