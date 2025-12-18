#ifndef LOCATION_H
#define LOCATION_H

#include "common.h"
#include "account.h"
#include <strings.h>

#define LOCATION_FILE_PATH "TCP_Server/data/location.txt"
#define MAX_CATEGORY_ENUM 10 // Số lượng tối đa các category tự định nghĩa
#define MAX_LOCATIONS 100 // Số lượng địa điểm tối đa mà Server
#define MAX_LOC_NAME 100 // Độ dài tối đa cho Tên địa điểm (name)
#define MAX_LOC_ADDR 200 // Độ dài tối đa cho Địa chỉ (address)
#define MAX_LOC_CAT 50 // Độ dài tối đa cho category (category)
#define MAX_LOC_DESC 255 // Độ dài tối đa cho Mô tả (description)

// 1. Các lệnh yêu cầu từ Client (Request Commands)
#define REQ_ADD_LOCATION    "ADD_LOCATION"
#define REQ_GET_LOCATIONS   "GET_LOCATIONS"

// 2. Các thông báo phản hồi từ Server (Response Messages)
#define MSG_ADD_SUCCESS     "Add location successful"
#define MSG_NO_CATEGORY     "No category found"
#define MSG_LOC_FOUND       "Found locations"
#define MSG_INVALID_FORMAT  "Invalid format. Use: name|addr|cat|desc"
#define MSG_DB_ERROR        "Internal server error"
#define MSG_END_DATA        "End of data"

/**
 * Structure to represent a location entity
 * Stored in file as: location_id|user_id|name|address|category|description
 */
typedef struct {
    int location_id;
    int user_id;                // ID of the user who created this location
    char name[MAX_LOC_NAME];
    char address[MAX_LOC_ADDR];
    char category[MAX_LOC_CAT]; // e.g., restaurant, cafe, cinema
    char description[MAX_LOC_DESC];
} Location;

// Global variables
extern Location locations[MAX_LOCATIONS];
extern int location_count;

extern char categories[MAX_CATEGORY_ENUM][MAX_LOC_CAT];

int load_locations(const char *filename);
int save_locations(const char *filename);
void handle_add_location(int client_index, char *args);
void handle_get_locations(int client_index, char *category);
int check_category_valid(char *category);

#endif