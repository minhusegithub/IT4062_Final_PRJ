#include "friend.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Get list of friends for current user
 * @param sock Socket descriptor
 */
void do_get_friends_list(int sock) {
    char response[MAX_LINE];

    // Send GET_FRIENDS command (no parameters)
    send_to_server(sock, REQ_GET_FRIENDS);

    if (receive_line(sock, response, sizeof(response)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return;
    }

    printf("Server send: %s\n", response);
}

/**
 * Unfriend a user by their ID
 * @param sock Socket descriptor
 */
void do_unfriend(int sock) {
    char id_str[20], out[MAX_LINE], response[MAX_LINE];
    
    printf("\n--- Unfriend ---\n");
    printf("Enter Friend's User ID to unfriend: ");
    if (!fgets(id_str, sizeof(id_str), stdin)) return;
    trim(id_str); 

    if (strlen(id_str) == 0) {
        printf("ID cannot be empty\n");
        return;
    }

    // Format: UNFRIEND <id>
    snprintf(out, sizeof(out), "%s %s", REQ_UNFRIEND, id_str);
    send_to_server(sock, out);

    if (receive_line(sock, response, sizeof(response)) > 0) {
        printf("Server: %s\n", response);
    }
}