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

/**
 * Get list of friend requests for current user
 * @param sock Socket descriptor
 */
void do_get_friend_requests(int sock) {
    char response[MAX_LINE];

    // Gửi lệnh GET_FRIEND_REQUESTS (không có tham số)
    send_to_server(sock, REQ_GET_FRIEND_REQUESTS);

    if (receive_line(sock, response, sizeof(response)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return;
    }

    printf("Server send: %s\n", response);
}

/**
 * Accept a friend request
 * @param sock Socket descriptor
 */
void do_accept_friend_request(int sock) {
    char user_id_str[20], out[MAX_LINE], response[MAX_LINE];
    
    printf("\n--- Accept Friend Request ---\n");
    printf("Enter user_id to accept friend request: ");
    if (!fgets(user_id_str, sizeof(user_id_str), stdin)) {
        printf("Error reading input\n");
        return;
    }
    
    // Trim trailing CR/LF from stdin
    trim_CRLF(user_id_str);
    
    if (strlen(user_id_str) == 0) {
        printf("User ID cannot be empty\n");
        return;
    }
    
    // Format: ACCEPT_FRIEND_REQUEST <user_id>
    int n = snprintf(out, sizeof(out), "%s %s", REQ_ACCEPT_FRIEND_REQUEST, user_id_str);
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

/**
 * Reject a friend request
 * @param sock Socket descriptor
 */
void do_reject_friend_request(int sock) {
    char user_id_str[20], out[MAX_LINE], response[MAX_LINE];
    
    printf("\n--- Reject Friend Request ---\n");
    printf("Enter user_id to reject friend request: ");
    if (!fgets(user_id_str, sizeof(user_id_str), stdin)) {
        printf("Error reading input\n");
        return;
    }
    
    // Trim trailing CR/LF from stdin
    trim_CRLF(user_id_str);
    
    if (strlen(user_id_str) == 0) {
        printf("User ID cannot be empty\n");
        return;
    }
    
    // Format: REJECT_FRIEND_REQUEST <user_id>
    int n = snprintf(out, sizeof(out), "%s %s", REQ_REJECT_FRIEND_REQUEST, user_id_str);
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

