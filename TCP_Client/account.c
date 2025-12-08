#include "account.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int do_login(int sock) {
    char username[MAX_USERNAME], password[MAX_PASSWORD], out[MAX_LINE], response[MAX_LINE];
    
    printf("Enter username: ");
    if (!fgets(username, sizeof(username), stdin)) return -1;
    printf("Enter password: ");
    if (!fgets(password, sizeof(password), stdin)) return -1;

    /* trim trailing CR/LF from stdin */
    trim_CRLF(username);
    trim_CRLF(password);
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("Username and password cannot be empty\n");
        return -1;
    }
    
    int n = snprintf(out, sizeof(out), "%s %s|%s", REQ_LOGIN, username, password); 
    if (n < 0 || (size_t)n >= sizeof(out)) {
        fprintf(stderr, "Input too long.\n");
        return -1;
    }
    
    send_to_server(sock,out);
    
    if (receive_line(sock, response, sizeof(response)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return -1;
    }
    
    printf("Server send: %s\n", response);
    return (atoi(response) == 110 );
}

void do_register(int sock){
    char username[512], password[512], out[MAX_LINE], response[MAX_LINE];
    
    printf("Enter username to register: ");
    if (!fgets(username, sizeof(username), stdin)) return;
    printf("Enter password to register: ");
    if (!fgets(password, sizeof(password), stdin)) return;

    /* trim trailing CR/LF from stdin */
    trim_CRLF(username);
    trim_CRLF(password);
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("Username and password cannot be empty\n");
        return;
    }
    
    int n = snprintf(out, sizeof(out), "%s %s|%s", REQ_REGISTER, username, password); // format the message to send to server
    if (n < 0 || (size_t)n >= sizeof(out)) {
        fprintf(stderr, "Input too long.\n");
        return;
    }
    
    send_to_server(sock,out);
    
    if (receive_line(sock, response, sizeof(response)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return;
    }
    
    printf("Server send: %s\n", response);
    return;
}

int do_logout(int sock) {
    char line[MAX_LINE];
    const char *out = REQ_LOGOUT; // format the message to send to server
    
    
    send_to_server(sock,out);

    if (receive_line(sock, line, sizeof(line)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return -1;
    }
    
    printf("%s\n", line);
    int code = atoi(line);

    return (code == 130) ? 1 : 0;
}