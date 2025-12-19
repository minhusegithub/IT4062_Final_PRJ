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
    trim(name);

    printf("Address: ");
    if (!fgets(addr, sizeof(addr), stdin))
        return;
    trim(addr);

    printf("Category (restaurant/cafe/cinema/fashion/other): ");
    if (!fgets(cat, sizeof(cat), stdin))
        return;
    trim(cat);

    printf("Description: ");
    if (!fgets(desc, sizeof(desc), stdin))
        return;
    trim(desc);

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
    // 1. Hiển thị gợi ý danh mục
    printf("Available Categories:\n");
    printf("  - restaurant\n");
    printf("  - cafe\n");
    printf("  - cinema\n");
    printf("  - fashion\n");
    printf("  - other\n");

    printf("Enter category to filter (leave empty for all): ");

    if (fgets(cat, sizeof(cat), stdin))
    {
        trim(cat);
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
    if (code != 110)
    {
        // Không thành công (220, 221, ...): in xong quay lại menu
        return;
    }

    /* Ví dụ mess nhận được từ server
    110 Found locations (2):
    110 1. Highlands Coffee Bach Khoa - 1 Dai Co Viet, Hai Ba Trung, HN (cafe)
    110 2. Pho Thin Lo Duc - 13 Lo Duc, Hai Ba Trung, HN (restaurant)
    110 /r/n */

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

/**
 * Display UI and send update location request to server
 * @param sock Socket descriptor for server communication
 */
void do_update_location(int sock)
{
    char id_str[20], name[100], addr[200], cat[50], desc[200];
    char out[MAX_LINE], response[MAX_LINE];

    printf("\n--- Update Location ---\n");
    
    // 1. Nhập ID địa điểm cần sửa
    printf("Enter Location ID to update: ");
    if (!fgets(id_str, sizeof(id_str), stdin)) return;
    trim(id_str);
    if (strlen(id_str) == 0) return;

    // 2. Nhập các thông tin mới
    printf("New Name: ");
    if (!fgets(name, sizeof(name), stdin)) return;
    trim(name);

    printf("New Address: ");
    if (!fgets(addr, sizeof(addr), stdin)) return;
    trim(addr);

    printf("New Category (restaurant/cafe/cinema/fashion/other): ");
    if (!fgets(cat, sizeof(cat), stdin)) return;
    trim(cat);

    printf("New Description: ");
    if (!fgets(desc, sizeof(desc), stdin)) return;
    trim(desc);

    // Kiểm tra dữ liệu rỗng (Name và Address bắt buộc)
    if (strlen(name) == 0 || strlen(addr) == 0)
    {
        printf("Error: Name and Address cannot be empty.\n");
        return;
    }

    // 3. Đóng gói gói tin theo format: UPDATE_LOCATION id|name|addr|cat|desc
    int n = snprintf(out, sizeof(out), "%s %s|%s|%s|%s|%s", 
             REQ_UPDATE_LOCATION, id_str, name, addr, cat, desc);
    
    if (n < 0 || (size_t)n >= sizeof(out)) {
        printf("Error: Input too long.\n");
        return;
    }

    // 4. Gửi lên server và nhận phản hồi
    send_to_server(sock, out);

    if (receive_line(sock, response, sizeof(response)) > 0)
    {
        printf("Server: %s\n", response);
    }
}

/**
 * Display UI and send delete location request to server
 * @param sock Socket descriptor for server communication
 */
void do_delete_location(int sock)
{
    char id_str[20];
    char out[MAX_LINE], response[MAX_LINE];

    printf("\n--- Delete Location ---\n");
    
    // 1. Nhập ID địa điểm cần xóa
    printf("Enter Location ID to delete: ");
    if (!fgets(id_str, sizeof(id_str), stdin)) return;
    trim(id_str);

    if (strlen(id_str) == 0)
    {
        printf("Error: ID cannot be empty.\n");
        return;
    }

    // 2. Đóng gói gói tin theo format: DELETE_LOCATION id
    snprintf(out, sizeof(out), "%s %s", REQ_DELETE_LOCATION, id_str);

    // 3. Gửi lên server và nhận phản hồi
    send_to_server(sock, out);

    if (receive_line(sock, response, sizeof(response)) > 0)
    {
        printf("Server: %s\n", response);
    }
}

/**
 * Send request to view locations created by the current user
 * @param sock Socket descriptor
 */
void do_view_my_locations(int sock)
{
    char out[MAX_LINE];
    char response[4096];

    printf("\n--- My Locations ---\n");

    // Gửi lệnh VIEW_MY_LOCATIONS (không cần tham số)
    snprintf(out, sizeof(out), "%s", REQ_VIEW_MY_LOCATIONS);
    send_to_server(sock, out);

    // Nhận dòng đầu tiên (Tiêu đề hoặc mã lỗi)
    if (receive_line(sock, response, sizeof(response)) <= 0)
        return;
    
    // In phản hồi từ server (VD: "Your locations (3): ...")
    printf("%s\n", response);

    int code = atoi(response);
    if (code != 110) return;

    // Vòng lặp nhận danh sách chi tiết
    while (1)
    {
        int n = receive_line(sock, response, sizeof(response));
        if (n <= 0) break;

        if (strstr(response, MSG_END_DATA) != NULL)
        {
            break;
        }
        printf("%s\n", response);
    }
}