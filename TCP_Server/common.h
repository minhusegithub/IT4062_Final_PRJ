#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define MAX_LINE 1024

#define MSG_INVALID_COMMAND "Invalid command"

// Client structure
typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    int user_id;
    int is_logged_in; // 0: not logged in, 1: logged in
} Client;

// Global clients array (defined in server.c)
extern Client clients[FD_SETSIZE];

// Function declarations
void send_reply_sock(int sockfd, int code, const char *msg);

#endif // COMMON_H

