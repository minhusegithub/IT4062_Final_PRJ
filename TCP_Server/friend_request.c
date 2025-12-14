#include "friend_request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables
FriendRequestList friend_requests[MAX_FRIEND_REQUESTS];
int friend_request_count = 0;

/**
 * Load friend requests from file
 * Format: user_id|n|user_id_1 user_id_2 ... user_id_n
 * @param filename Path to friend request file
 * @return 0 on success, -1 on error
 */
int load_friend_requests(const char *filename) {
    FILE *file = fopen(filename, "r"); // read
    if (file == NULL) {
        return 0;
    }
    
    friend_request_count = 0;
    char line[4096];
    
    while (fgets(line, sizeof(line), file) != NULL && friend_request_count < MAX_FRIEND_REQUESTS) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = '\0';
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Parse line: user_id|n|user_id_1 user_id_2 ... user_id_n
        char *token;
        
        // Parse user_id
        token = strtok(line, "|");
        if (token == NULL) continue;
        friend_requests[friend_request_count].user_id = atoi(token);
        
        // Parse count n
        token = strtok(NULL, "|");
        if (token == NULL) continue;
        int count = atoi(token);
        friend_requests[friend_request_count].request_count = count;
        
        // Parse list of user_ids
        token = strtok(NULL, "|");
        if (token == NULL) {
            friend_requests[friend_request_count].request_count = 0;
            friend_request_count++;
            continue;
        }
        
        // Copy token to a temporary buffer for parsing (strtok modifies the string)
        char id_list[1024];
        strncpy(id_list, token, sizeof(id_list) - 1);
        id_list[sizeof(id_list) - 1] = '\0';
        
        // Parse space-separated user_ids
        int idx = 0;
        char *id_token = strtok(id_list, " ");
        while (id_token != NULL && idx < MAX_FRIEND_REQUESTS_PER_USER && idx < count) {
            friend_requests[friend_request_count].request_ids[idx] = atoi(id_token);
            idx++;
            id_token = strtok(NULL, " ");
        }
        
        friend_requests[friend_request_count].request_count = idx;
        friend_request_count++;
    }
    
    fclose(file);
    return 0;
}

/**
 * Save friend requests to file
 * Format: user_id|n|user_id_1 user_id_2 ... user_id_n
 * @param filename Path to friend request file
 * @return 0 on success, -1 on error
 */
int save_friend_requests(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening friend request file for writing");
        return -1;
    }
    
    for (int i = 0; i < friend_request_count; i++) {
        fprintf(file, "%d|%d|", 
                friend_requests[i].user_id,
                friend_requests[i].request_count);
        
        // Write list of user_ids
        for (int j = 0; j < friend_requests[i].request_count; j++) {
            if (j > 0) fprintf(file, " ");
            fprintf(file, "%d", friend_requests[i].request_ids[j]);
        }
        fprintf(file, "\n");
    }
    
    fflush(file);
    fclose(file);
    return 0;
}

/**
 * Find friend request list for a specific user_id
 * @param user_id User ID to search for
 * @return Pointer to FriendRequestList if found, NULL otherwise
 */
FriendRequestList* find_friend_request_list(int user_id) {
    for (int i = 0; i < friend_request_count; i++) {
        if (friend_requests[i].user_id == user_id) {
            return &friend_requests[i];
        }
    }
    return NULL;
}

/**
 * Check if from_user_id has already sent request to to_user_id
 * @param from_user_id User ID who sent the request
 * @param to_user_id User ID who received the request
 * @return 1 if already sent, 0 otherwise
 */
int has_sent_request(int from_user_id, int to_user_id) {
    FriendRequestList *list = find_friend_request_list(to_user_id);
    if (list == NULL) {
        return 0;
    }
    
    for (int i = 0; i < list->request_count; i++) {
        if (list->request_ids[i] == from_user_id) {
            return 1;
        }
    }
    return 0;
}

/**
 * Handle SEND_FRIEND_REQUEST command
 * @param client_index Client index in clients array
 * @param args Arguments containing username
 */
void handle_send_friend_request(int client_index, char *args) {
    Client *client = &clients[client_index];
    
    // Check if user is logged in
    check_login(client_index);
    
    // Parse username from args
    if (args == NULL || strlen(args) == 0) {
        send_reply_sock(client->socket_fd, 300, MSG_INVALID_COMMAND);
        return;
    }
    
    // Trim whitespace
    char username[MAX_USERNAME];
    strncpy(username, args, MAX_USERNAME - 1);
    username[MAX_USERNAME - 1] = '\0';
    
    // Trim trailing spaces
    int len = strlen(username);
    while (len > 0 && username[len - 1] == ' ') {
        username[len - 1] = '\0';
        len--;
    }
    
    if (strlen(username) == 0) {
        send_reply_sock(client->socket_fd, 300, MSG_INVALID_COMMAND);
        return;
    }
    
    // Find target user by username
    Account *target_account = find_account(username);
    if (target_account == NULL) {
        send_reply_sock(client->socket_fd, 210, MSG_USER_NOT_FOUND);
        return;
    }
    
    int from_user_id = client->user_id;
    int to_user_id = target_account->user_id;
    
    // Check if sending to self
    if (from_user_id == to_user_id) {
        send_reply_sock(client->socket_fd, 404, MSG_SELF_REQUEST);
        return;
    }
    
    // Check if already sent request
    if (has_sent_request(from_user_id, to_user_id)) {
        send_reply_sock(client->socket_fd, 403, MSG_ALREADY_SENT);
        return;
    }
    
    // Find or create friend request list for target user
    FriendRequestList *list = find_friend_request_list(to_user_id);
    
    if (list == NULL) {
        // Create new record
        if (friend_request_count >= MAX_FRIEND_REQUESTS) {
            send_reply_sock(client->socket_fd, 400, "Server error: too many friend requests");
            return;
        }
        
        friend_requests[friend_request_count].user_id = to_user_id;
        friend_requests[friend_request_count].request_count = 1;
        friend_requests[friend_request_count].request_ids[0] = from_user_id;
        friend_request_count++;
    } else {
        // Add to existing list
        if (list->request_count >= MAX_FRIEND_REQUESTS_PER_USER) {
            send_reply_sock(client->socket_fd, 405, MSG_TOO_MANY_REQUEST_PER_USER);
            return;
        }
        
        list->request_ids[list->request_count] = from_user_id;
        list->request_count++;
    }
    
    // Save to file
    if (save_friend_requests(FRIEND_REQUEST_FILE_PATH) == 0) {
        send_reply_sock(client->socket_fd, 120, MSG_FRIEND_REQUEST_SUCCESS);
    } else {
        send_reply_sock(client->socket_fd, 400, "Server error: failed to save");
    }
}

