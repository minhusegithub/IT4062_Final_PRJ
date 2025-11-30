#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME 100
#define MAX_PASSWORD 100
#define MAX_ACCOUNT 5000
#define ACCOUNT_FILE_PATH "TCP_Server/data/account.txt"

// Account structure: user_id|username|password||status
typedef struct {
    int user_id;
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    int status;  // 0: inactive, 1: active
} Account;

// Global variables
extern Account accounts[MAX_ACCOUNT];
extern int account_count;

// Function declarations
int load_accounts(const char *filename);
Account* find_account(const char *username);


#endif // ACCOUNT_H

