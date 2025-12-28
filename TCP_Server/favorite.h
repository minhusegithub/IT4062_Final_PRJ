#ifndef FAVORITE_H
#define FAVORITE_H

#include "common.h"
#include "location.h"

#define FAVORITE_FILE_PATH "TCP_Server/data/favourite_location.txt"
#define MAX_FAVORITES_PER_USER 100
#define MAX_FAV_LISTS 1024

// Protocol commands
#define REQ_SAVE_TO_FAV_LOCATION "SAVE_TO_FAV_LOCATION"
#define REQ_VIEW_FAVORITE_LOCATIONS "VIEW_FAVORITE_LOCATIONS"

// Response messages
#define MSG_SAVE_FAV_SUCCESS "Save to favorite location successful"
#define MSG_ALREADY_FAV "Location is already in your favorites"
#define MSG_FAV_FULL "Favorite list is full"
#define MSG_FAV_READ_SUCCESS "Get favorite locations successfully"
#define MSG_NO_FAVORITE "You have no favorite locations"

/**
 * Structure to store favorite locations for a user
 * Format in file: user_id|n|loc_id1 loc_id2 ... loc_idn
 */
typedef struct {
    int user_id;
    int count;
    int location_ids[MAX_FAVORITES_PER_USER];
} FavoriteList;

// Global variables
extern FavoriteList favorites[MAX_FAV_LISTS];
extern int favorite_count;

// Function declarations
int load_favorites(const char *filename);
int save_favorites(const char *filename);
void handle_save_favorite(int client_index, char *args);
void handle_view_favorite_locations(int client_index);

#endif