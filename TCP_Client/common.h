#ifndef CLIENT_COMMON_H
#define CLIENT_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

void trim_CRLF(char *str);
int receive_line(int sockfd, char *buf, size_t bufsz);
void send_reply_sock(int sockfd , const char *msg);
 
#endif // CLIENT_COMMON_H