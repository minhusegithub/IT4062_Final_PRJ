#ifndef CLIENT_ACCOUNT_H
#define CLIENT_ACCOUNT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define MAX_USERNAME 100
#define MAX_PASSWORD 100
#define MAX_LINE 1024



#define REQ_REGISTER "REGISTER"
#define REQ_LOGOUT "LOGOUT"
#define REQ_LOGIN "LOGIN"



// Function declarations
int do_login(int sock);
void do_register(int sock);
int do_logout(int sock);

#endif // CLIENT_ACCOUNT_H