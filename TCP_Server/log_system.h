#ifndef LOG_SYSTEM_H
#define LOG_SYSTEM_H

#include "common.h"
#include "account.h"
#include <time.h>

#define LOG_FILE_PATH "TCP_Server/data/server_log.txt"

// Function declarations
void log_activity(int client_index, const char *action, const char *details);
void log_system(const char *message);

#endif