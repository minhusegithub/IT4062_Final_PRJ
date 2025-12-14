#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define MAX_USERNAME 100
#define MAX_PASSWORD 100
#define MAX_ACCOUNT 1024
#define ACCOUNT_FILE_PATH "TCP_Server/data/account.txt"

#define MSG_REGISTER_SUCCESS "Register successful"
#define MSG_USERNAME_EXISTS "Username already exists"
#define MSG_LOGOUT_SUCCESS "Logout successful"
#define MSG_NEED_LOGIN "Need to login first"
#define MSG_LOGIN_SUCCESS "Login successful"
#define MSG_LOGIN_LOCKED "Account is locked"
#define MSG_LOGIN_ALREADY "Already logged in"
#define MSG_LOGIN_NOT_FOUND "Account not found"
#define MSG_WRONG_PASSWORD "Wrong password"

#define REQ_REGISTER "REGISTER"
#define REQ_LOGOUT "LOGOUT"
#define REQ_LOGIN "LOGIN"

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
void handle_register(int client_index,  char *args );
void handle_logout(int client_index ,  char *args);
void handle_login(int client_index,  char *args );
void check_login(int client_index);

#endif // ACCOUNT_H

