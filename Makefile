CC = gcc
CFLAGS = -Wall -Wextra -std=c11
SERVER_DIR = TCP_Server
CLIENT_DIR = TCP_Client

.PHONY: all clean server client

all: server client

server:
	$(CC) $(CFLAGS) -o server $(SERVER_DIR)/server.c $(SERVER_DIR)/account.c

client:
	$(CC) $(CFLAGS) -o client $(CLIENT_DIR)/client.c

clean:
	rm -f server client

