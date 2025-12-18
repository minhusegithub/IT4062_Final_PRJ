#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables
Account accounts[MAX_ACCOUNT];
int account_count = 0;

/**
 * Load accounts from account.txt file
 * Format: user_id|username|password||status
 * @param filename Path to account file
 * @return 0 on success, -1 on error
 */
int load_accounts(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // File doesn't exist, create it
        file = fopen(filename, "w");
        if (file == NULL) {
            perror("Error creating account file");
            return -1;
        }
        fclose(file);
        account_count = 0;
        return 0;
    }
    
    account_count = 0;
    char line[1024];
    
    while (fgets(line, sizeof(line), file) != NULL && account_count < MAX_ACCOUNT) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = '\0';
        
        // Skip empty lines
        if (strlen(line) == 0) {
            continue;
        }
        
        // Parse line: user_id|username|password|status
        char *token;
        
        // Parse user_id
        token = strtok(line, "|");
        if (token == NULL) continue;
        accounts[account_count].user_id = atoi(token);
        
        // Parse username
        token = strtok(NULL, "|");
        if (token == NULL) continue;
        strncpy(accounts[account_count].username, token, MAX_USERNAME - 1);
        accounts[account_count].username[MAX_USERNAME - 1] = '\0';
        
        // Parse password
        token = strtok(NULL, "|");
        if (token == NULL) continue;
        strncpy(accounts[account_count].password, token, MAX_PASSWORD - 1);
        accounts[account_count].password[MAX_PASSWORD - 1] = '\0';
        
        // Parse status
        token = strtok(NULL, "|");
        if (token == NULL) {
            // Default status to 1 if not provided
            accounts[account_count].status = 1;
        } else {
            accounts[account_count].status = atoi(token);
        }
        
        account_count++;
    }
    
    fclose(file);
    return 0;
}

/**
 * Find account by username
 * @param username Username to search for
 * @return Pointer to Account if found, NULL otherwise
 */
Account* find_account(const char *username) {
    if (username == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < account_count; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            return &accounts[i];
        }
    }
    
    return NULL;
}

/**
 * Handle register request
 * @param client_index Client index in clients array
 * @param msg Message received from client
 */
void handle_register(int client_index,  char *args ){
    Client *client = &clients[client_index];
    
  
    // Tách các tham số từ chuỗi args (username|password|)
    char *username = strtok(args, "|");
    char *password = strtok(NULL, " ");
    

    // Check if username already exists
    if (find_account(username) != NULL) {
        send_reply_sock(client->socket_fd, 400, MSG_USERNAME_EXISTS);
        return ;
    }
    
    // Generate new user_id (increment from highest existing ID)
    int new_user_id = account_count + 1;
    
    // Add to in-memory array
    accounts[account_count].user_id = new_user_id;
    strncpy(accounts[account_count].username, username, MAX_USERNAME - 1);
    accounts[account_count].username[MAX_USERNAME - 1] = '\0';
    strncpy(accounts[account_count].password, password, MAX_PASSWORD - 1);
    accounts[account_count].password[MAX_PASSWORD - 1] = '\0';
    accounts[account_count].status = 1; // Default status: active
    
    // Append to file
    FILE *file = fopen(ACCOUNT_FILE_PATH, "a");
    if (file == NULL) {
        // Remove from array if file write fails
        account_count--;
        return ;
    }
    
    // Write in format: user_id|username|password|status
    fprintf(file, "%d|%s|%s|%d\n", 
            accounts[account_count].user_id,
            accounts[account_count].username,
            accounts[account_count].password,
            accounts[account_count].status);
    
    fclose(file);
    account_count++;
    send_reply_sock(client->socket_fd, 130, MSG_REGISTER_SUCCESS);
    return;
}

/**
 * Handle logout request
 * @param client_index Client index in clients array
 */
void handle_logout(int client_index ,  char *args) {
    Client *client = &clients[client_index];
    (void)args; // Suppress unused parameter warning
    check_login(client_index);
    
    client->is_logged_in = 0;
    client->user_id = -1;
    send_reply_sock(client->socket_fd, 130, MSG_LOGOUT_SUCCESS);
}

/**
 * Handle login request
 * @param client_index Client index in clients array
 * @param msg Message received from client
 */
void handle_login(int client_index,  char *args ) {
    Client *client = &clients[client_index];
    
    // Tách các tham số từ chuỗi args (username|password|)
    char *username = strtok(args, "|");
    char *password = strtok(NULL, "");

    if (client->is_logged_in) {
        send_reply_sock(client->socket_fd, 213, MSG_LOGIN_ALREADY);
        return;
    }
            
    Account *account = find_account(username);
    if (account == NULL) {
        send_reply_sock(client->socket_fd, 212, MSG_LOGIN_NOT_FOUND);
        return;
    }
    
    if (strcmp(account->password, password) != 0) {
        send_reply_sock(client->socket_fd, 214, MSG_WRONG_PASSWORD);
        return;
    }
    
    if (account->status == 0) {
        send_reply_sock(client->socket_fd, 211, MSG_LOGIN_LOCKED);
        return;
    }
    
    client->user_id = account->user_id;
    client->is_logged_in = 1;

    send_reply_sock(client->socket_fd, 110, MSG_LOGIN_SUCCESS);
}


void check_login(int client_index){
    if (clients[client_index].is_logged_in == 0) {    
        send_reply_sock(clients[client_index].socket_fd, 221, MSG_NEED_LOGIN);
        return;
    }
}