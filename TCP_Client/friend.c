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
