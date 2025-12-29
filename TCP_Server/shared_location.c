#include "shared_location.h"
#include "friend_request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log_system.h"

SharedList shared_lists[MAX_SHARED_LISTS];
int shared_count = 0;

/**
 * Load shared locations from file
 * Format: user_id|n|sharer_id1 loc_id1 sharer_id2 loc_id2 ...
 * @param filename Path to shared location file
 * @return 0 on success, -1 on error
 */
int load_shared_locations(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        file = fopen(filename, "w"); // Tạo mới nếu chưa có
        if (!file) return -1;
        fclose(file);
        return 0;
    }

    shared_count = 0;
    char line[4096];
    while (fgets(line, sizeof(line), file) && shared_count < MAX_SHARED_LISTS) {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        char *token = strtok(line, "|");
        if (!token) continue;
        shared_lists[shared_count].user_id = atoi(token);

        token = strtok(NULL, "|");
        if (!token) continue;
        int n = atoi(token);
        shared_lists[shared_count].count = n;

        // Đọc danh sách items: sharer_id loc_id sharer_id loc_id ...
        token = strtok(NULL, "|");
        if (token) {
            char temp[2048];
            strncpy(temp, token, sizeof(temp)-1);
            
            char *sub = strtok(temp, " ");
            int idx = 0;
            while (sub && idx < n) {
                shared_lists[shared_count].items[idx].sharer_id = atoi(sub);
                sub = strtok(NULL, " ");
                if (sub) {
                    shared_lists[shared_count].items[idx].location_id = atoi(sub);
                    sub = strtok(NULL, " ");
                }
                idx++;
            }
            shared_lists[shared_count].count = idx;
        } else {
             shared_lists[shared_count].count = 0;
        }
        shared_count++;
    }
    fclose(file);
    return 0;
}

/**
 * Save shared locations to file
 * @param filename Path to shared location file
 * @return 0 on success, -1 on error
 */
int save_shared_locations(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return -1;

    for (int i = 0; i < shared_count; i++) {
        fprintf(file, "%d|%d|", shared_lists[i].user_id, shared_lists[i].count);
        for (int j = 0; j < shared_lists[i].count; j++) {
            if (j > 0) fprintf(file, " ");
            fprintf(file, "%d %d", shared_lists[i].items[j].sharer_id, shared_lists[i].items[j].location_id);
        }
        fprintf(file, "\n");
    }
    fclose(file);
    return 0;
}

/**
 * Find shared list for a specific user
 * @param user_id User ID to search for
 * @return Pointer to SharedList if found, NULL otherwise
 */
SharedList* find_shared_list(int user_id) {
    for (int i = 0; i < shared_count; i++) {
        if (shared_lists[i].user_id == user_id) return &shared_lists[i];
    }
    return NULL;
}

/**
 * Handle SHARE_LOCATION request from client
 * Command: SHARE_LOCATION <location_name>|<username_friend>
 * @param client_index Index of the client in the global clients array
 * @param args Argument string containing location name and friend's username
 */
void handle_share_location(int client_index, char *args) {
    // 1. Kiểm tra đăng nhập
    if (check_login(client_index) == 0) {
        return;
    }

    // 2. Tách tham số: location_name và username
    char *loc_name = strtok(args, "|");
    char *friend_name = strtok(NULL, "|");

    if (!loc_name || !friend_name) {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_FORMAT);
        return;
    }

    int my_id = clients[client_index].user_id;

    // 3. Tìm account của người bạn (người được tag)
    Account *acc_friend = find_account(friend_name);
    if (!acc_friend) {
        send_reply_sock(clients[client_index].socket_fd, 210, MSG_USER_NOT_FOUND);
        return;
    }
    int friend_id = acc_friend->user_id;

    // 4. Kiểm tra xem username kia có phải là bạn bè không
    FriendList *flist = find_friend_list(my_id);
    int is_friend = 0;
    if (flist) {
        for(int i=0; i < flist->friend_count; i++) {
            if (flist->friend_ids[i] == friend_id) {
                is_friend = 1; break;
            }
        }
    }
    if (!is_friend) {
        send_reply_sock(clients[client_index].socket_fd, 406, MSG_NOT_FRIEND_SHARE);
        return;
    }

    // 5. Tìm địa điểm theo tên
    int loc_id = -1;
    for (int i=0; i<location_count; i++) {
        if (strcasecmp(locations[i].name, loc_name) == 0) {
            loc_id = locations[i].location_id;
            break;
        }
    }
    if (loc_id == -1) {
        send_reply_sock(clients[client_index].socket_fd, 220, MSG_LOC_NAME_NOT_FOUND);
        return;
    }

    // 6. Thêm vào danh sách được chia sẻ của người bạn (friend_id)
    SharedList *slist = find_shared_list(friend_id);
    
    // Nếu chưa có danh sách, tạo mới
    if (!slist) {
        if (shared_count >= MAX_SHARED_LISTS) {
            send_reply_sock(clients[client_index].socket_fd, 400, MSG_DB_ERROR);
            return;
        }
        slist = &shared_lists[shared_count++];
        slist->user_id = friend_id;
        slist->count = 0;
    }

    // Kiểm tra danh sách đầy chưa
    if (slist->count >= MAX_SHARED_PER_USER) {
         send_reply_sock(clients[client_index].socket_fd, 400, MSG_SHARED_LIST_FULL);
         return;
    }

    // Thêm item mới
    slist->items[slist->count].sharer_id = my_id;
    slist->items[slist->count].location_id = loc_id;
    slist->count++;

    // 7. Lưu file và phản hồi thành công (Mã 120)
    save_shared_locations(SHARED_FILE_PATH);

    // 8. Ghi log hoạt động
    char log_msg[200];
    snprintf(log_msg, sizeof(log_msg), "Shared LocID %d to User %s", loc_id, friend_name);
    log_activity(client_index, "SHARE_LOCATION", log_msg);

    send_reply_sock(clients[client_index].socket_fd, 120, MSG_SHARE_SUCCESS);
}

/**
 * Handle GET_SHARED_LOCATIONS request from client
 * Command: GET_SHARED_LOCATIONS
 * @param client_index Index of the client in the global clients array
 */
void handle_get_shared_locations(int client_index) {
    // 1. Kiểm tra đăng nhập
    if (check_login(client_index) == 0) {
        return;
    }

    int my_id = clients[client_index].user_id;
    SharedList *slist = find_shared_list(my_id);

    // 2. Kiểm tra danh sách rỗng
    if (!slist || slist->count == 0) {
        send_reply_sock(clients[client_index].socket_fd, 220, MSG_NO_SHARED_LOC);
        return;
    }

    char buffer[4096] = "";
    char line[512];
    int found_count = 0;

    // 3. Duyệt danh sách và format dữ liệu
    for (int i = 0; i < slist->count; i++) {
        int sharer_id = slist->items[i].sharer_id;
        int loc_id = slist->items[i].location_id;

        // Tìm tên người share
        char sharer_name[100] = "Unknown";
        for(int k=0; k<account_count; k++) {
            if(accounts[k].user_id == sharer_id) {
                strcpy(sharer_name, accounts[k].username);
                break;
            }
        }

        // Tìm thông tin địa điểm và format
        for(int m=0; m<location_count; m++) {
            if(locations[m].location_id == loc_id) {
                // Format: ID. Name - Address (Shared by ...)
                snprintf(line, sizeof(line), "\n%d. %s - %s (Shared by %s)", 
                         locations[m].location_id, locations[m].name, locations[m].address, sharer_name);
                
                if (strlen(buffer) + strlen(line) < sizeof(buffer) - 1) {
                    strcat(buffer, line);
                    found_count++;
                }
                break;
            }
        }
    }

    // 4. Gửi phản hồi
    if (found_count > 0) {
        char header[100];
        snprintf(header, sizeof(header), "%s (%d):", MSG_SHARED_READ_SUCCESS, found_count);
        
        char *final_msg = malloc(strlen(header) + strlen(buffer) + 1);
        if (final_msg) {
            sprintf(final_msg, "%s%s", header, buffer);
            send_reply_sock(clients[client_index].socket_fd, 130, final_msg);
            free(final_msg);
        }
        // Gửi tín hiệu kết thúc danh sách
        send_reply_sock(clients[client_index].socket_fd, 130, MSG_END_DATA);
    } else {
        send_reply_sock(clients[client_index].socket_fd, 220, MSG_NO_SHARED_LOC);
    }
}