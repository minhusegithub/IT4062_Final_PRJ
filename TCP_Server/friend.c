#include "friend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables
FriendList friends[MAX_FRIENDS];
int friend_count = 0;

/**
 * Load friends from file
 * Format: user_id|n|user_id_1 user_id_2 ... user_id_n
 * @param filename Path to friend file
 * @return 0 on success, -1 on error
 */
int load_friends(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // File doesn't exist, create it
        file = fopen(filename, "w");
        if (file == NULL) {
            perror("Error creating friend file");
            return -1;
        }
        fclose(file);
        friend_count = 0;
        return 0;
    }
    
    friend_count = 0;
    char line[4096];
    
    while (fgets(line, sizeof(line), file) != NULL && friend_count < MAX_FRIENDS) {
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
        friends[friend_count].user_id = atoi(token);
        
        // Parse count n
        token = strtok(NULL, "|");
        if (token == NULL) continue;
        int count = atoi(token);
        friends[friend_count].friend_count = count;
        
        // Parse list of user_ids
        token = strtok(NULL, "|");
        if (token == NULL) {
            friends[friend_count].friend_count = 0;
            friend_count++;
            continue;
        }
        
        // Copy token to a temporary buffer for parsing (strtok modifies the string)
        char id_list[1024];
        strncpy(id_list, token, sizeof(id_list) - 1);
        id_list[sizeof(id_list) - 1] = '\0';
        
        // Parse space-separated user_ids
        int idx = 0;
        char *id_token = strtok(id_list, " ");
        while (id_token != NULL && idx < MAX_FRIENDS_PER_USER && idx < count) {
            friends[friend_count].friend_ids[idx] = atoi(id_token);
            idx++;
            id_token = strtok(NULL, " ");
        }
        
        friends[friend_count].friend_count = idx;
        friend_count++;
    }
    
    fclose(file);
    return 0;
}

/**
 * Save friends to file
 * Format: user_id|n|user_id_1 user_id_2 ... user_id_n
 * @param filename Path to friend file
 * @return 0 on success, -1 on error
 */
int save_friends(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Error opening friend file for writing");
        return -1;
    }
    
    for (int i = 0; i < friend_count; i++) {
        fprintf(file, "%d|%d|", 
                friends[i].user_id,
                friends[i].friend_count);
        
        // Write list of user_ids
        for (int j = 0; j < friends[i].friend_count; j++) {
            if (j > 0) fprintf(file, " ");
            fprintf(file, "%d", friends[i].friend_ids[j]);
        }
        fprintf(file, "\n");
    }
    
    fflush(file);
    fclose(file);
    return 0;
}

/**
 * Find friend list for a specific user_id
 * @param user_id User ID to search for
 * @return Pointer to FriendList if found, NULL otherwise
 */
FriendList* find_friend_list(int user_id) {
    for (int i = 0; i < friend_count; i++) {
        if (friends[i].user_id == user_id) {
            return &friends[i];
        }
    }
    return NULL;
}

/**
 * Add a friend to user's friend list
 * @param user_id User ID who will have the friend
 * @param friend_id Friend's user ID to add
 * @return 0 on success, -1 if already friends, -2 if too many friends
 */
int add_friend(Client *client, int user_id, int friend_id) {
    // Check if already friends
    FriendList *list = find_friend_list(user_id);
    if (list != NULL) {
        for (int i = 0; i < list->friend_count; i++) {
            if (list->friend_ids[i] == friend_id) {
                send_reply_sock(client->socket_fd, 402, MSG_ALREADY_FRIENDS);
                return -1; 
            }
        }
        
        // Add to existing list
        if (list->friend_count >= MAX_FRIENDS_PER_USER) {
            send_reply_sock(client->socket_fd, 405, MSG_TOO_MANY_FRIENDS);
            return -2;
        }
        
        list->friend_ids[list->friend_count] = friend_id;
        list->friend_count++;
    } else {
        // Create new record
        friends[friend_count].user_id = user_id;
        friends[friend_count].friend_count = 1;
        friends[friend_count].friend_ids[0] = friend_id;
        friend_count++;
    }
    
    return 0;
}

/**
 * Handle GET_FRIENDS command
 * @param client_index Client index in clients array
 */
void handle_get_friends(int client_index) {
    Client *client = &clients[client_index];
    
    // Check login (221 if not logged in)
    if (check_login(client_index) == 0) {
        return;
    }
    
    int user_id = client->user_id;
    
    // Find friend list for current user
    FriendList *list = find_friend_list(user_id);
    if (list == NULL || list->friend_count == 0) {
        send_reply_sock(client->socket_fd, 130, MSG_NO_FRIENDS);
        return;
    }
    
    char final_msg[4096 + strlen(MSG_GET_FRIENDS_SUCCESS) + 1];
    char buffer[4096] = "";
    char line[256];
    
    snprintf(buffer, sizeof(buffer), "%d %s:", list->friend_count, "friends");
    
    for (int i = 0; i < list->friend_count; i++) {
        int friend_user_id = list->friend_ids[i];
        
        // Find username corresponding to friend_user_id
        const char *username = "Unknown";
        for (int j = 0; j < account_count; j++) {
            if (accounts[j].user_id == friend_user_id) {
                username = accounts[j].username;
                break;
            }
        }
        
        snprintf(line, sizeof(line), "\n%d. %s (id=%d)", i + 1, username, friend_user_id);
        
        // Avoid buffer overflow
        if (strlen(buffer) + strlen(line) + 2 >= sizeof(buffer)) {
            break;
        }
        
        strcat(buffer, line);
    }
    
    snprintf(final_msg, sizeof(final_msg), "%s\n%s", MSG_GET_FRIENDS_SUCCESS, buffer);
    send_reply_sock(client->socket_fd, 130, final_msg);
}

