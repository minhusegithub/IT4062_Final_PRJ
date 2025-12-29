#include "log_system.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h> // Cho inet_ntoa

/**
 * Get current time string formatted as [YYYY-MM-DD HH:MM:SS]
 * @param buffer Buffer to store the result string
 * @param size Size of the buffer
 */
void get_current_time_str(char *buffer, size_t size) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, size, "[%Y-%m-%d %H:%M:%S]", timeinfo);
}

/**
 * Log client activity to file and console
 * @param client_index Index of the client in the global clients array
 * @param action Short string describing the action (e.g., "LOGIN", "ADD_LOC")
 * @param details Additional details about the action
 */
void log_activity(int client_index, const char *action, const char *details) {
    // 1. Mở file log ở chế độ append (ghi tiếp)
    FILE *file = fopen(LOG_FILE_PATH, "a");
    if (!file) {
        perror("Cannot open log file");
        return;
    }

    // 2. Lấy thời gian hiện tại
    char time_str[64];
    get_current_time_str(time_str, sizeof(time_str));

    // 3. Lấy thông tin IP và Port của client
    char *client_ip = inet_ntoa(clients[client_index].address.sin_addr);
    int client_port = ntohs(clients[client_index].address.sin_port);

    // 4. Lấy thông tin User (Username/ID) nếu đã đăng nhập
    char user_info[256];
    if (clients[client_index].is_logged_in) {
        char username[100] = "Unknown";
        // Tìm username trong danh sách accounts
        for(int i = 0; i < account_count; i++) {
            if (accounts[i].user_id == clients[client_index].user_id) {
                strcpy(username, accounts[i].username);
                break;
            }
        }
        snprintf(user_info, sizeof(user_info), "%s(ID:%d)", username, clients[client_index].user_id);
    } else {
        strcpy(user_info, "Guest");
    }

    // 5. Ghi log vào file theo định dạng: [Time] [IP:Port] [User] Action: Details
    fprintf(file, "%s [%s:%d] [%s] %s: %s\n", 
            time_str, client_ip, client_port, user_info, action, details ? details : "");

    // 6. In ra màn hình console server để theo dõi trực tiếp
    printf("%s [%s:%d] [%s] %s: %s\n", 
            time_str, client_ip, client_port, user_info, action, details ? details : "");

    // 7. Đóng file
    fclose(file);
}

/**
 * Log system events (server start, stop, errors)
 * @param message The system message to log
 */
void log_system(const char *message) {
    // 1. Mở file log
    FILE *file = fopen(LOG_FILE_PATH, "a");
    if (!file) return;

    // 2. Lấy thời gian hiện tại
    char time_str[64];
    get_current_time_str(time_str, sizeof(time_str));

    // 3. Ghi log hệ thống
    fprintf(file, "%s [SYSTEM] %s\n", time_str, message);
    printf("%s [SYSTEM] %s\n", time_str, message);

    // 4. Đóng file
    fclose(file);
}