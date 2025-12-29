CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SERVER_DIR = TCP_Server
CLIENT_DIR = TCP_Client

.PHONY: all clean server client

all: server client

server:
	$(CC) $(CFLAGS) -o server $(SERVER_DIR)/server.c $(SERVER_DIR)/account.c $(SERVER_DIR)/location.c $(SERVER_DIR)/friend_request.c $(SERVER_DIR)/friend.c $(SERVER_DIR)/favorite.c $(SERVER_DIR)/shared_location.c $(SERVER_DIR)/log_system.c

client:
	$(CC) $(CFLAGS) -o client $(CLIENT_DIR)/client.c $(CLIENT_DIR)/account.c $(CLIENT_DIR)/location.c $(CLIENT_DIR)/friend_request.c $(CLIENT_DIR)/friend.c $(CLIENT_DIR)/common.c $(CLIENT_DIR)/shared_location.c

clean:
	rm -f server client