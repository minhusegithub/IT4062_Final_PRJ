#include "location.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Biến toàn cục để lưu danh sách địa điểm
Location locations[MAX_LOCATIONS];
int location_count = 0;

/**
 * Save current locations from memory back to the text file
 * @param filename Path to the location data file
 * @return 0 on success, -1 on error
 */
int save_locations(const char *filename) {
    FILE *file = fopen(filename, "w"); // ghi đè từ mảng locations vào file text
    if (!file) {
        perror("Error opening location file for writing");
        return -1;
    }
    
    // Duyệt qua mảng và ghi từng dòng theo định dạng chuẩn
    for (int i = 0; i < location_count; i++) {
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
int load_locations(const char *filename) {
    // Mở file để đọc
    FILE *file = fopen(filename, "r");
    if (!file) {
        // Nếu file chưa tồn tại, tạo file mới rồi đóng lại ngay
        file = fopen(filename, "w");
        if (!file) return -1;
        fclose(file);
        return 0;
    }

    location_count = 0;
    char line[1024];
    
    // Đọc từng dòng cho đến khi hết file hoặc đầy mảng
    while (fgets(line, sizeof(line), file) && location_count < MAX_LOCATIONS) {
        // Xóa ký tự xuống dòng ở cuối chuỗi đọc được
        line[strcspn(line, "\r\n")] = 0;

        // Bắt đầu tách chuỗi dựa trên ký tự phân cách '|'
        char *token = strtok(line, "|");
        if (!token) continue;
        locations[location_count].location_id = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        locations[location_count].user_id = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        // Copy dữ liệu vào struct, đảm bảo không tràn bộ nhớ
        strncpy(locations[location_count].name, token, MAX_LOC_NAME - 1);

        token = strtok(NULL, "|");
        if (!token) continue;
        strncpy(locations[location_count].address, token, MAX_LOC_ADDR - 1);

        token = strtok(NULL, "|");
        if (!token) continue;
        strncpy(locations[location_count].category, token, MAX_LOC_CAT - 1);

        token = strtok(NULL, "|");
        if (!token) continue;
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
void handle_add_location(int client_index, char *args) {
    // 1. Kiểm tra xem user đã đăng nhập chưa
    if (clients[client_index].is_logged_in == 0) {
        send_reply_sock(clients[client_index].socket_fd, 221, MSG_NEED_LOGIN);
        return;
    }

    // 2. Tách các tham số từ chuỗi args (Name|Address|Category|Description)
    char *name = strtok(args, "|");
    char *addr = strtok(NULL, "|");
    char *cat = strtok(NULL, "|");
    char *desc = strtok(NULL, "|");

    // Kiểm tra tính hợp lệ của dữ liệu đầu vào
    if (!name || !addr || !cat || !desc) {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_FORMAT);
        return;
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
    if (save_locations(LOCATION_FILE_PATH) == 0) {
        send_reply_sock(clients[client_index].socket_fd, 120, MSG_ADD_SUCCESS);
    } else {
        send_reply_sock(clients[client_index].socket_fd, 400, MSG_DB_ERROR);
    }
}

/**
 * Handle the GET_LOCATIONS request from a client
 * @param client_index Index of the client in the global clients array
 * @param category The category to filter by (optional)
 */
void handle_get_locations(int client_index, char *category) {
    
    // Kiểm tra xem user đã đăng nhập chưa
    if (clients[client_index].is_logged_in == 0) {
        send_reply_sock(clients[client_index].socket_fd, 221, MSG_NEED_LOGIN);
        return;
    }

    char buffer[4096] = ""; // Buffer chứa toàn bộ danh sách
    char line_buff[512];
    int found = 0; // Đếm số địa điểm tìm thấy

    // Duyệt qua tất cả địa điểm hiện có
    for (int i = 0; i < location_count; i++) {
        // Kiểm tra điều kiện lọc: Nếu category rỗng (lấy hết) hoặc trùng khớp
        int match = 0;
        if (category == NULL || strlen(category) == 0) {
            match = 1;
        } else {
            if (strcmp(locations[i].category, category) == 0) {
                match = 1;
            }
        }

        if (match) {
            // Định dạng dòng hiển thị: ID. Name - Address (Category)
            snprintf(line_buff, sizeof(line_buff), "\n%d. %s - %s (%s)", 
                    locations[i].location_id, 
                    locations[i].name, 
                    locations[i].address, 
                    locations[i].category);
            
            // Kiểm tra xem buffer có bị tràn không trước khi nối chuỗi
            if (strlen(buffer) + strlen(line_buff) < sizeof(buffer) - 1) {
                strcat(buffer, line_buff);
                found++;
            } else {
                break; // Dừng nếu buffer đầy
            }
        }
    }

    if (found == 0) {
        send_reply_sock(clients[client_index].socket_fd, 220, MSG_NO_LOCATIONS);
    } else {
        // Gửi mã thành công (110) kèm theo số lượng và danh sách
        char header[100];
        snprintf(header, sizeof(header), "%s (%d):", MSG_LOC_FOUND, found); // ví dụ: "Found locations (5):"
        
        // Cấp phát bộ nhớ động để nối header và nội dung danh sách
        char *final_msg = malloc(strlen(header) + strlen(buffer) + 1);
        if (final_msg) {
            sprintf(final_msg, "%s%s", header, buffer);
            send_reply_sock(clients[client_index].socket_fd, 110, final_msg);
            free(final_msg);
        }

        send_reply_sock(clients[client_index].socket_fd, 110, MSG_END_DATA);
    }
}