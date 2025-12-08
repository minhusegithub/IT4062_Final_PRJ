#include "common.h"
#include "location.h"
#include <stdio.h>
#include <string.h>

/**
 * Handle add location UI and request
 * @param sock Socket descriptor
 */
void do_add_location(int sock)
{
    char name[100], addr[200], cat[50], desc[200];
    char out[MAX_LINE], response[MAX_LINE];

    printf("\n--- Add New Location ---\n");

    printf("Name: ");
    if (!fgets(name, sizeof(name), stdin))
        return;
    trim_CRLF(name);

    printf("Address: ");
    if (!fgets(addr, sizeof(addr), stdin))
        return;
    trim_CRLF(addr);

    printf("Category (restaurant/cafe/cinema/fashion): ");
    if (!fgets(cat, sizeof(cat), stdin))
        return;
    trim_CRLF(cat);

    printf("Description: ");
    if (!fgets(desc, sizeof(desc), stdin))
        return;
    trim_CRLF(desc);

    if (strlen(name) == 0 || strlen(addr) == 0)
    {
        printf("Error: Name and Address cannot be empty.\n");
        return;
    }

    // Format: ADD_LOCATION name|address|cat|desc
    int n = snprintf(out, sizeof(out), "%s %s|%s|%s|%s", REQ_ADD_LOCATION, name, addr, cat, desc);
    if (n < 0 || (size_t)n >= sizeof(out))
    {
        printf("Error: Input too long.\n");
        return;
    }

    send_to_server(sock, out);
    if (receive_line(sock, response, sizeof(response)) > 0)
    {
        printf("Server: %s\n", response);
    }
}

/**
 * Handle get locations UI and request
 * @param sock Socket descriptor
 */
void do_get_locations(int sock)
{
    char cat[50], out[MAX_LINE];
    char response[4096]; // Large buffer for list

    printf("\n--- List Locations ---\n");
    printf("Enter category to filter (leave empty for all): ");

    if (fgets(cat, sizeof(cat), stdin))
    {
        trim_CRLF(cat);
    }
    else
    {
        cat[0] = '\0';
    }

    // Format: GET_LOCATIONS category
    snprintf(out, sizeof(out), "%s %s", REQ_GET_LOCATIONS, cat);
    send_to_server(sock, out);

    // 1. Đọc dòng đầu tiên
    if (receive_line(sock, response, sizeof(response)) <= 0)
        return;
    printf("%s\n", response);

    int code = atoi(response);
    if (code != 110) {
        // Không thành công (220, 221, ...): in xong quay lại menu
        return;
    }

    /* Ví dụ mess nhận được từ server 
    110 Found locations (2):
    110 1. Highlands Coffee Bach Khoa - 1 Dai Co Viet, Hai Ba Trung, HN (cafe)
    110 2. Pho Thin Lo Duc - 13 Lo Duc, Hai Ba Trung, HN (restaurant)
    110 END_DATA */

    while (1)
    {
        int n = receive_line(sock, response, sizeof(response));
        if (n <= 0)
            break;

        if (strstr(response, MSG_END_DATA) != NULL)
        {
            break;
        }

        printf("%s\n", response);
    }
}