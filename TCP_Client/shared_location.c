#include "shared_location.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Handle SHARE_LOCATION UI and request
 * @param sock Socket descriptor
 */
void do_share_location(int sock) {
    char loc_name[100], friend_name[100];
    char out[MAX_LINE], response[MAX_LINE];

    printf("\n--- Share Location ---\n");
    printf("Enter Location Name to share: ");
    if (!fgets(loc_name, sizeof(loc_name), stdin)) return;
    trim(loc_name);

    printf("Enter Friend's Username to tag: ");
    if (!fgets(friend_name, sizeof(friend_name), stdin)) return;
    trim(friend_name);

    if (strlen(loc_name) == 0 || strlen(friend_name) == 0) {
        printf("Error: Fields cannot be empty.\n");
        return;
    }

    // Format: SHARE_LOCATION <loc_name>|<friend_name>
    snprintf(out, sizeof(out), "%s %s|%s", REQ_SHARE_LOCATION, loc_name, friend_name);
    send_to_server(sock, out);

    if (receive_line(sock, response, sizeof(response)) > 0) {
        printf("Server: %s\n", response);
    }
}

/**
 * Handle GET_SHARED_LOCATIONS UI and request
 * @param sock Socket descriptor
 */
void do_get_shared_locations(int sock) {
    char out[MAX_LINE], response[4096];
    
    printf("\n--- Shared With Me ---\n");
    // Gửi lệnh không tham số
    snprintf(out, sizeof(out), "%s", REQ_GET_SHARED_LOCATIONS);
    send_to_server(sock, out);

    // Đọc dòng đầu tiên (Header hoặc lỗi)
    if (receive_line(sock, response, sizeof(response)) <= 0) return;
    printf("%s\n", response);

    int code = atoi(response);
    if (code != 130) return;

    // Đọc danh sách chi tiết
    while(1) {
        int n = receive_line(sock, response, sizeof(response));
        if (n <= 0) break;
        if (strstr(response, "End of data") != NULL) break;
        printf("%s\n", response);
    }
}