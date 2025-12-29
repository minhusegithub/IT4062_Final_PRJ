#include "location.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_system.h"

// Biến toàn cục để lưu danh sách địa điểm
Location locations[MAX_LOCATIONS];
int location_count = 0;
// Biến toàn cục để lưu enum categories
char categories[MAX_CATEGORY_ENUM][MAX_LOC_CAT] = {"restaurant", "cafe", "cinema", "fashion", "other"};

/**
 * Save current locations from memory back to the text file
 * @param filename Path to the location data file
 * @return 0 on success, -1 on error
 */
int save_locations(const char *filename)
{
    FILE *file = fopen(filename, "w"); // ghi đè từ mảng locations vào file text
    if (!file)
    {
        perror("Error opening location file for writing");
        return -1;
    }

    // Duyệt qua mảng và ghi từng dòng theo định dạng chuẩn
    for (int i = 0; i < location_count; i++)
    {
        fprintf(file, "%d|%d|%s|%s|%s|%s\n",
                locations[i].location_id,
                locations[i].user_id,
                locations[i].name,
                locations[i].address,
                locations[i].category,
                locations[i].description);
    }
    fflush(file);
    fclose(file);
    return 0;
}

/**
 * Load locations from the text file into memory
 * @param filename Path to the location data file
 * @return 0 on success, -1 on error
 */
int load_locations(const char *filename)
{
    // Mở file để đọc
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        // Nếu file chưa tồn tại, tạo file mới rồi đóng lại ngay
        file = fopen(filename, "w");
        if (!file)
            return -1;
        fclose(file);
        return 0;
    }

    location_count = 0;
    char line[1024];

    // Đọc từng dòng cho đến khi hết file hoặc đầy mảng
    while (fgets(line, sizeof(line), file) && location_count < MAX_LOCATIONS)
    {
        // Xóa ký tự xuống dòng ở cuối chuỗi đọc được
        line[strcspn(line, "\r\n")] = 0;

        // Bắt đầu tách chuỗi dựa trên ký tự phân cách '|'
        char *token = strtok(line, "|");
        if (!token)
            continue;
        locations[location_count].location_id = atoi(token);

        token = strtok(NULL, "|");
        if (!token)
            continue;
        locations[location_count].user_id = atoi(token);

        token = strtok(NULL, "|");
        if (!token)
            continue;
        // Copy dữ liệu vào struct, đảm bảo không tràn bộ nhớ
        strncpy(locations[location_count].name, token, MAX_LOC_NAME - 1);

        token = strtok(NULL, "|");
        if (!token)
            continue;
        strncpy(locations[location_count].address, token, MAX_LOC_ADDR - 1);

        token = strtok(NULL, "|");
        if (!token)
            continue;
        strncpy(locations[location_count].category, token, MAX_LOC_CAT - 1);

        token = strtok(NULL, "|");
        if (!token)
            continue;
        strncpy(locations[location_count].description, token, MAX_LOC_DESC - 1);

        location_count++;
    }
    fclose(file);
    return 0;
}

/**
 * Handle the ADD_LOCATION request from a client
 * @param client_index Index of the client in the global clients array
 * @param args The argument string containing location details separated by '|'
 */
void handle_add_location(int client_index, char *args)
{

    // 1. Kiểm tra xem user đã đăng nhập chưa
    if (check_login(client_index) == 0)
    {
        return;
    }

    // 2. Tách các tham số từ chuỗi args (Name|Address|Category|Description)
    char *name = strtok(args, "|");
    char *addr = strtok(NULL, "|");
    char *cat = strtok(NULL, "|");
    char *desc = strtok(NULL, "|");

    // Kiểm tra tính hợp lệ của dữ liệu đầu vào
    if (!name || !addr || !cat || !desc)
    {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_FORMAT);
        return;
    }

    // Chuẩn hóa danh mục (category)
    int found = 0; // Biến kiểm tra category người dùng nhập có nằm trong danh sách enum ko
    for (int i = 0; i < MAX_CATEGORY_ENUM; i++)
    {
        // hết danh sách -> dừng vòng lặp
        if (categories[i][0] == '\0')
        {
            break;
        }

        // kiểm tra không phân biệt hoa thường
        if (strcasecmp(cat, categories[i]) == 0)
        {
            found = 1;
            strcpy(cat, categories[i]);
            break; // Tìm thấy rồi thì thoát
        }
    }
    if (found == 0)
    {
        strcpy(cat, "other");
    }

    // 3. Tạo ID mới (ID lớn nhất hiện tại + 1)
    int new_id = (location_count > 0) ? locations[location_count - 1].location_id + 1 : 1;

    // Gán dữ liệu vào mảng trong bộ nhớ
    locations[location_count].location_id = new_id;
    locations[location_count].user_id = clients[client_index].user_id;
    strncpy(locations[location_count].name, name, MAX_LOC_NAME - 1);
    strncpy(locations[location_count].address, addr, MAX_LOC_ADDR - 1);
    strncpy(locations[location_count].category, cat, MAX_LOC_CAT - 1);
    strncpy(locations[location_count].description, desc, MAX_LOC_DESC - 1);

    location_count++;

    // 4. Lưu ngay xuống file để đảm bảo dữ liệu không bị mất
    if (save_locations(LOCATION_FILE_PATH) == 0)
    {
        log_activity(client_index, "ADD_LOCATION", name);
        send_reply_sock(clients[client_index].socket_fd, 120, MSG_ADD_SUCCESS);
    }
    else
    {
        send_reply_sock(clients[client_index].socket_fd, 400, MSG_DB_ERROR);
    }
}

/**
 * Handle the GET_LOCATIONS request from a client
 * @param client_index Index of the client in the global clients array
 * @param category The category to filter by (optional)
 */
void handle_get_locations(int client_index, char *category)
{

    // Kiểm tra xem user đã đăng nhập chưa
    if (check_login(client_index) == 0)
    {
        return;
    }

    if (!check_category_valid(category))
    {
        send_reply_sock(clients[client_index].socket_fd, 220, MSG_NO_CATEGORY);
        return;
    }

    char buffer[4096] = ""; // Buffer chứa toàn bộ danh sách
    char line_buff[512];
    int found = 0; // Đếm số địa điểm tìm thấy

    // Duyệt qua tất cả địa điểm hiện có
    for (int i = 0; i < location_count; i++)
    {
        // Kiểm tra điều kiện lọc: Nếu category rỗng (lấy hết) hoặc trùng khớp
        int match = 0;
        if (category == NULL || strlen(category) == 0)
        {
            match = 1;
        }
        else
        {
            if (strcmp(locations[i].category, category) == 0)
            {
                match = 1;
            }
        }

        if (match)
        {
            // Định dạng dòng hiển thị: ID. Name - Address (Category)
            snprintf(line_buff, sizeof(line_buff), "\n%d. %s - %s (%s)",
                     locations[i].location_id,
                     locations[i].name,
                     locations[i].address,
                     locations[i].category);

            // Kiểm tra xem buffer có bị tràn không trước khi nối chuỗi
            if (strlen(buffer) + strlen(line_buff) < sizeof(buffer) - 1)
            {
                strcat(buffer, line_buff);
                found++;
            }
            else
            {
                break; // Dừng nếu buffer đầy
            }
        }
    }

    // Gửi mã thành công (110) kèm theo số lượng và danh sách
    char header[100];
    snprintf(header, sizeof(header), "%s (%d):", MSG_LOC_FOUND, found); // ví dụ: "Found locations (5):"

    // Cấp phát bộ nhớ động để nối header và nội dung danh sách
    char *final_msg = malloc(strlen(header) + strlen(buffer) + 1);
    if (final_msg)
    {
        sprintf(final_msg, "%s%s", header, buffer);
        send_reply_sock(clients[client_index].socket_fd, 110, final_msg);
        free(final_msg);
    }

    send_reply_sock(clients[client_index].socket_fd, 110, MSG_END_DATA);
}

/**
 * Check category is valid in enum categories
 * @param char *categories category to check if is valid
 */
int check_category_valid(char *category)
{
    if (category == NULL || strlen(category) == 0)
    {
        return 2;
    }

    for (int i = 0; i < MAX_CATEGORY_ENUM; i++)
    {
        if (strcmp(category, categories[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/**
 * Handle UPDATE_LOCATION request from client
 * @param client_index Index of the client in the global clients array
 * @param args Argument string containing location details to update (format: id|name|addr|cat|desc)
 */
void handle_update_location(int client_index, char *args)
{
    // 1. Kiểm tra đăng nhập
    if (check_login(client_index) == 0)
    {
        return;
    }

    // 2. Tách các tham số: id|name|addr|cat|desc
    char *token = strtok(args, "|");
    if (!token)
    {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_FORMAT);
        return;
    }
    int loc_id = atoi(token);

    char *name = strtok(NULL, "|");
    char *addr = strtok(NULL, "|");
    char *cat = strtok(NULL, "|");
    char *desc = strtok(NULL, "|");

    if (!name || !addr || !cat || !desc)
    {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_FORMAT);
        return;
    }

    // 3. Tìm địa điểm trong danh sách hiện có
    int found_idx = -1;
    for (int i = 0; i < location_count; i++)
    {
        if (locations[i].location_id == loc_id)
        {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1)
    {
        send_reply_sock(clients[client_index].socket_fd, 220, MSG_LOC_NOT_FOUND);
        return;
    }

    // 4. Kiểm tra quyền (Chỉ người tạo mới được sửa)
    if (locations[found_idx].user_id != clients[client_index].user_id)
    {
        send_reply_sock(clients[client_index].socket_fd, 222, MSG_NO_PERMISSION);
        return;
    }

    // 5. Kiểm tra và chuẩn hóa Category (Danh mục)
    int found_cat = 0;
    for (int i = 0; i < MAX_CATEGORY_ENUM; i++)
    {
        if (categories[i][0] == '\0')
            break;

        // So sánh không phân biệt hoa thường
        if (strcasecmp(cat, categories[i]) == 0)
        {
            found_cat = 1;
            strcpy(cat, categories[i]); // Chuẩn hóa về chữ thường theo enum
            break;
        }
    }
    // Nếu không khớp danh mục nào, gán là "other"
    if (!found_cat)
    {
        strcpy(cat, "other");
    }

    // 6. Cập nhật dữ liệu vào bộ nhớ
    strncpy(locations[found_idx].name, name, MAX_LOC_NAME - 1);
    strncpy(locations[found_idx].address, addr, MAX_LOC_ADDR - 1);
    strncpy(locations[found_idx].category, cat, MAX_LOC_CAT - 1);
    strncpy(locations[found_idx].description, desc, MAX_LOC_DESC - 1);

    // 7. Lưu lại xuống file
    if (save_locations(LOCATION_FILE_PATH) == 0)
    {
        char log_msg[100];
        snprintf(log_msg, sizeof(log_msg), "Updated LocID: %d", locations[found_idx].location_id);
        log_activity(client_index, "UPDATE_LOCATION", log_msg);
        send_reply_sock(clients[client_index].socket_fd, 130, MSG_UPDATE_SUCCESS);
    }
    else
    {
        send_reply_sock(clients[client_index].socket_fd, 400, MSG_DB_ERROR);
    }
}

/**
 * Handle DELETE_LOCATION request from client
 * @param client_index Index of the client in the global clients array
 * @param args Argument string containing ID of the location to delete
 */
void handle_delete_location(int client_index, char *args)
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

    // 3. Tìm địa điểm trong danh sách
    int found_idx = -1;
    for (int i = 0; i < location_count; i++)
    {
        if (locations[i].location_id == loc_id)
        {
            found_idx = i;
            break;
        }
    }

    if (found_idx == -1)
    {
        send_reply_sock(clients[client_index].socket_fd, 220, MSG_LOC_NOT_FOUND);
        return;
    }

    // 4. Kiểm tra quyền (Chỉ người tạo mới được xóa)
    if (locations[found_idx].user_id != clients[client_index].user_id)
    {
        send_reply_sock(clients[client_index].socket_fd, 222, MSG_NO_PERMISSION);
        return;
    }

    // 5. Xóa khỏi bộ nhớ
    for (int i = found_idx; i < location_count - 1; i++)
    {
        locations[i] = locations[i + 1];
    }
    location_count--;

    // 6. Lưu danh sách mới xuống file
    if (save_locations(LOCATION_FILE_PATH) == 0)
    {
        char log_msg[100];
        snprintf(log_msg, sizeof(log_msg), "Deleted LocID: %d", loc_id);
        log_activity(client_index, "DELETE_LOCATION", log_msg);

        send_reply_sock(clients[client_index].socket_fd, 140, MSG_DELETE_SUCCESS);
    }
    else
    {
        send_reply_sock(clients[client_index].socket_fd, 400, MSG_DB_ERROR);
    }
}

/**
 * Handle VIEW_MY_LOCATIONS request (List locations created by current user)
 * @param client_index Index of the client in the global clients array
 */
void handle_view_my_locations(int client_index)
{
    if (check_login(client_index) == 0)
    {
        return;
    }

    char buffer[4096] = "";
    char line_buff[512];
    int found = 0;
    int current_uid = clients[client_index].user_id;

    // Duyệt qua tất cả địa điểm
    for (int i = 0; i < location_count; i++)
    {
        // Lọc: Chỉ lấy địa điểm có user_id trùng với người đang gọi
        if (locations[i].user_id == current_uid)
        {
            snprintf(line_buff, sizeof(line_buff), "\n%d. %s - %s (%s)",
                     locations[i].location_id,
                     locations[i].name,
                     locations[i].address,
                     locations[i].category);

            if (strlen(buffer) + strlen(line_buff) < sizeof(buffer) - 1)
            {
                strcat(buffer, line_buff);
                found++;
            }
            else
            {
                break; // Buffer đầy
            }
        }
    }

    // Gửi phản hồi
    char header[100];
    if (found > 0)
    {
        snprintf(header, sizeof(header), "Your locations (%d):", found);
    }
    else
    {
        snprintf(header, sizeof(header), "You haven't added any locations yet.");
    }

    char *final_msg = malloc(strlen(header) + strlen(buffer) + 1);
    if (final_msg)
    {
        sprintf(final_msg, "%s%s", header, buffer);
        send_reply_sock(clients[client_index].socket_fd, 110, final_msg);
        free(final_msg);
    }

    // Gửi tín hiệu kết thúc dữ liệu
    send_reply_sock(clients[client_index].socket_fd, 110, MSG_END_DATA);
}
