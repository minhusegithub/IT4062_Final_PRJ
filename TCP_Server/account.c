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



