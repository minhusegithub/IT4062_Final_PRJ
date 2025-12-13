#include "friend_request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Send friend request to a user
 * @param sock Socket descriptor
 */
void do_send_friend_request(int sock) {
    char username[100], out[MAX_LINE], response[MAX_LINE];
    
    printf("\n--- Send Friend Request ---\n");
    printf("Enter username to send friend request: ");
    if (!fgets(username, sizeof(username), stdin)) {
        printf("Error reading input\n");
        return;
    }
    
    // Trim trailing CR/LF from stdin
    trim_CRLF(username);
    
    if (strlen(username) == 0) {
        printf("Username cannot be empty\n");
        return;
    }
    
    // Format: SEND_FRIEND_REQUEST <username>
    int n = snprintf(out, sizeof(out), "%s %s", REQ_SEND_FRIEND_REQUEST, username);
    if (n < 0 || (size_t)n >= sizeof(out)) {
        fprintf(stderr, "Input too long.\n");
        return;
    }
    
    send_to_server(sock, out);
    
    if (receive_line(sock, response, sizeof(response)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return;
    }
    
    printf("Server send: %s\n", response);
    
    
}

