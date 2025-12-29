#include "favorite.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_system.h"

// Global variables definition
FavoriteList favorites[MAX_FAV_LISTS];
int favorite_count = 0;

/**
 * Load favorite lists from file
 * Format: user_id|n|loc_id1 loc_id2 ... loc_idn
 * @param filename Path to favorite file
 * @return 0 on success, -1 on error
 */
int load_favorites(const char *filename)
{
    FILE *file = fopen(filename, "r");
    // 1. Nếu file không tồn tại, tạo file mới
    if (!file)
    {
        file = fopen(filename, "w");
        if (!file)
            return -1;
        fclose(file);
        return 0;
    }

    favorite_count = 0;
    char line[4096];

    // 2. Đọc từng dòng từ file
    while (fgets(line, sizeof(line), file) && favorite_count < MAX_FAV_LISTS)
    {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0)
            continue;

        // 3. Phân tích cú pháp dòng: user_id|n|list_ids
        char *token = strtok(line, "|");
        if (!token)
            continue;
        favorites[favorite_count].user_id = atoi(token);

        token = strtok(NULL, "|");
        if (!token)
            continue;
        int n = atoi(token);
        favorites[favorite_count].count = n;

        // 4. Đọc danh sách ID địa điểm (nếu có)
        token = strtok(NULL, "|");
        if (token)
        {
            char temp_ids[2048];
            strncpy(temp_ids, token, sizeof(temp_ids) - 1);

            char *id_token = strtok(temp_ids, " ");
            int idx = 0;
            while (id_token && idx < n)
            {
                favorites[favorite_count].location_ids[idx++] = atoi(id_token);
                id_token = strtok(NULL, " ");
            }
            favorites[favorite_count].count = idx;
        }
        else
        {
            favorites[favorite_count].count = 0;
        }
        favorite_count++;
    }
    fclose(file);
    return 0;
}

/**
 * Save favorite lists to file
 * @param filename Path to favorite file
 * @return 0 on success, -1 on error
 */
int save_favorites(const char *filename)
{
    FILE *file = fopen(filename, "w");
    // 1. Kiểm tra mở file
    if (!file)
        return -1;

    // 2. Ghi từng record xuống file
    for (int i = 0; i < favorite_count; i++)
    {
        fprintf(file, "%d|%d|", favorites[i].user_id, favorites[i].count);

        // 3. Ghi danh sách ID cách nhau bởi khoảng trắng
        for (int j = 0; j < favorites[i].count; j++)
        {
            if (j > 0)
                fprintf(file, " ");
            fprintf(file, "%d", favorites[i].location_ids[j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
    return 0;
}

/**
 * Find favorite list for a specific user
 * @param user_id User ID to search for
 * @return Pointer to FavoriteList if found, NULL otherwise
 */
FavoriteList *find_favorite_list(int user_id)
{
    for (int i = 0; i < favorite_count; i++)
    {
        if (favorites[i].user_id == user_id)
            return &favorites[i];
    }
    return NULL;
}

/**
 * Handle SAVE_TO_FAV_LOCATION request from client
 * @param client_index Index of the client in the global clients array
 * @param args Argument string containing ID of the location to save
 */
void handle_save_favorite(int client_index, char *args)
{
    // 1. Kiểm tra đăng nhập
    if (check_login(client_index) == 0)
    {
        return;
    }

    // 2. Lấy ID địa điểm từ tham số
    if (!args || strlen(args) == 0)
    {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_FORMAT);
        return;
    }
    int loc_id = atoi(args);

    // 3. Kiểm tra địa điểm có tồn tại trong hệ thống không
    int loc_exists = 0;
    for (int i = 0; i < location_count; i++)
    {
        if (locations[i].location_id == loc_id)
        {
            loc_exists = 1;
            break;
        }
    }

    if (!loc_exists)
    {
        send_reply_sock(clients[client_index].socket_fd, 220, MSG_LOC_NOT_FOUND);
        return;
    }

    // 4. Tìm hoặc tạo danh sách yêu thích cho user hiện tại
    int user_id = clients[client_index].user_id;
    FavoriteList *list = find_favorite_list(user_id);

    if (list)
    {
        // 4a. Nếu đã có danh sách, kiểm tra xem đã lưu địa điểm này chưa
        for (int i = 0; i < list->count; i++)
        {
            if (list->location_ids[i] == loc_id)
            {
                send_reply_sock(clients[client_index].socket_fd, 402, MSG_ALREADY_FAV);
                return;
            }
        }
        // 4b. Kiểm tra danh sách đã đầy chưa
        if (list->count >= MAX_FAVORITES_PER_USER)
        {
            send_reply_sock(clients[client_index].socket_fd, 405, MSG_FAV_FULL);
            return;
        }
        // 4c. Thêm địa điểm vào danh sách
        list->location_ids[list->count++] = loc_id;
    }
    else
    {
        // 4d. Nếu chưa có danh sách, tạo mới

        // Nếu đã đạt giới hạn danh sách yêu thích
        if (favorite_count >= MAX_FAV_LISTS)
        {
            send_reply_sock(clients[client_index].socket_fd, 400, MSG_DB_ERROR);
            return;
        }

        favorites[favorite_count].user_id = user_id;
        favorites[favorite_count].count = 1;
        favorites[favorite_count].location_ids[0] = loc_id;
        favorite_count++;
    }

    // 5. Lưu xuống file và phản hồi kết quả
    if (save_favorites(FAVORITE_FILE_PATH) == 0)
    {
        char log_msg[100];
        snprintf(log_msg, sizeof(log_msg), "Saved LocID: %d", loc_id);
        log_activity(client_index, "SAVE_FAVORITE", log_msg);
        
        send_reply_sock(clients[client_index].socket_fd, 150, MSG_SAVE_FAV_SUCCESS);
    }
    else
    {
        send_reply_sock(clients[client_index].socket_fd, 400, MSG_DB_ERROR);
    }
}

/**
 * Handle VIEW_FAVORITE_LOCATIONS request
 * @param client_index Index of the client in the global clients array
 */
void handle_view_favorite_locations(int client_index)
{
    // 1. Kiểm tra đăng nhập
    if (check_login(client_index) == 0)
    {
        return;
    }

    int user_id = clients[client_index].user_id;
    FavoriteList *list = find_favorite_list(user_id);

    // 2. Kiểm tra xem có danh sách yêu thích không
    if (list == NULL || list->count == 0)
    {
        send_reply_sock(clients[client_index].socket_fd, 222, MSG_NO_FAVORITE);
        return;
    }

    char buffer[4096] = "";
    char line_buff[512];
    int found_count = 0;

    // 3. Duyệt qua danh sách ID yêu thích
    for (int i = 0; i < list->count; i++)
    {
        int loc_id = list->location_ids[i];

        // 4. Tìm chi tiết địa điểm trong mảng locations toàn cục
        // (Biến locations và location_count được extern từ location.h)
        for (int j = 0; j < location_count; j++)
        {
            if (locations[j].location_id == loc_id)
            {

                // Format: ID. Name - Address (Category)
                snprintf(line_buff, sizeof(line_buff), "\n%d. %s - %s (%s)",
                         locations[j].location_id,
                         locations[j].name,
                         locations[j].address,
                         locations[j].category);

                if (strlen(buffer) + strlen(line_buff) < sizeof(buffer) - 1)
                {
                    strcat(buffer, line_buff);
                    found_count++;
                }
                break;
            }
        }
    }

    // 5. Gửi phản hồi về Client
    if (found_count > 0)
    {
        char header[100];
        snprintf(header, sizeof(header), "%s (%d):", MSG_FAV_READ_SUCCESS, found_count);

        char *final_msg = malloc(strlen(header) + strlen(buffer) + 1);
        if (final_msg)
        {
            sprintf(final_msg, "%s%s", header, buffer);
            send_reply_sock(clients[client_index].socket_fd, 110, final_msg);
            free(final_msg);
        }
        // Gửi tín hiệu kết thúc danh sách
        send_reply_sock(clients[client_index].socket_fd, 110, MSG_END_DATA);
    }
    else
    {
        // Trường hợp có ID trong list nhưng không tìm thấy trong locations (ví dụ location đã bị xóa)
        send_reply_sock(clients[client_index].socket_fd, 222, MSG_NO_FAVORITE);
    }
}