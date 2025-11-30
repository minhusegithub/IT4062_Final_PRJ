#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include "account.h"

#define BUFFER_SIZE 1024
#define MAX_LINE 1024
// #define MAX_USERNAME 1000
// #define MAX_PASSWORD 1000
#define MAX_ACCOUNT 5000
#define BACKLOG 20

#define MSG_INVALID_COMMAND "Invalid command"
#define MSG_LOGIN_ALREADY "Already logged in"
#define MSG_LOGIN_NOT_FOUND "Account not found"
#define MSG_LOGIN_LOCKED "Account is locked"
#define MSG_LOGIN_SUCCESS "Login successful"
#define MSG_NEED_LOGIN "Need to login first"
#define MSG_LOGOUT_SUCCESS "Logout successful"




typedef struct {
    int socket_fd;
    struct sockaddr_in address;
    int user_id;
    int is_logged_in; // 0: not logged in, 1: logged in
} Client;

// list of clients - using array with FD_SETSIZE
Client clients[FD_SETSIZE];




/**
 * Remove trailing \r and \n characters from string
 * @param str String to process (will be modified)
 */
void trim_CRLF(char *str) {
    if (str == NULL) return;
    
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
}



/**
 * Read bytes from socket one by one until \n character is encountered
 * @param sockfd Socket descriptor to read from
 * @param buf Buffer to store read data
 * @param bufsz Buffer size
 * @return Length of line read, 0 if connection closed, -1 on error
 */
int receive_line(int sockfd, char *buf, size_t bufsz) {
    size_t idx = 0;
    while (idx < bufsz - 1) {
        ssize_t n = recv(sockfd, &buf[idx], 1, 0);
        if (n == 1) {
            if (buf[idx] == '\n') {
                buf[idx] = '\0';
                trim_CRLF(buf);
                return (int)strlen(buf);
            }
            idx++;
        } else if (n == 0) {
            return 0; /* peer closed */
        } else {
            if (errno == EINTR) continue;
            return -1;
        }
    }
    buf[bufsz-1] = '\0';
    trim_CRLF(buf);
    return (int)strlen(buf);
}

/**
 * Send reply to client with format "code message\r\n"
 * @param sockfd Client socket descriptor
 * @param code Response code (integer)
 * @param msg Message to send
 */
void send_reply_sock(int sockfd, int code, const char *msg) {
    char out[MAX_LINE];
    int n = snprintf(out, sizeof(out), "%d %s\r\n", code, msg);
    if (n <= 0) return;
    
    size_t total = (size_t)n; // total length of the message
    size_t sent = 0; // number of bytes sent
    
    while (sent < total) {
        ssize_t bytes_written = send(sockfd, out + sent, total - sent, 0);
        if (bytes_written > 0) {
            sent += (size_t)bytes_written;
        } else if (bytes_written < 0 && errno == EINTR) {
            continue; // if the send is interrupted, continue
        } else {
            break;
        }
    }
}

void handle_login(int client_index, const char *msg ) {
    Client *client = &clients[client_index];
    
    // get username and password from msg format LOGIN <username> <password>
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    
    // Parse message: LOGIN <username> <password>
    if (sscanf(msg, "LOGIN %s %s", username, password) != 2) {
        send_reply_sock(client->socket_fd, 300, MSG_INVALID_COMMAND);
        return;
    }

    if (client->is_logged_in) {
        send_reply_sock(client->socket_fd, 213, MSG_LOGIN_ALREADY);
        return;
    }
            
    Account *account = find_account(username);
    if (account == NULL) {
        send_reply_sock(client->socket_fd, 212, MSG_LOGIN_NOT_FOUND);
        return;
    }
    
    if (strcmp(account->password, password) != 0) {
        send_reply_sock(client->socket_fd, 214, "Wrong password");
        return;
    }
    
    if (account->status == 0) {
        send_reply_sock(client->socket_fd, 211, MSG_LOGIN_LOCKED);
        return;
    }
    
    client->user_id = account->user_id;
    client->is_logged_in = 1;

    send_reply_sock(client->socket_fd, 110, MSG_LOGIN_SUCCESS);
}

void handle_bye(int client_index) {
    Client *client = &clients[client_index];
    
    if (!client->is_logged_in) {
        send_reply_sock(client->socket_fd, 221, MSG_NEED_LOGIN);
        return;
    }
    
    // Đăng xuất thành công
    client->is_logged_in = 0;
    client->user_id = -1;
    send_reply_sock(client->socket_fd, 130, MSG_LOGOUT_SUCCESS);
}




void handle_register(int client_index, const char *msg ){
    Client *client = &clients[client_index];
    
    // get username and password from msg format LOGIN <username> <password>
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    
    // Parse message: REGISTER <username> <password>
    if (sscanf(msg, "REGISTER %s %s", username, password) != 2) {
        send_reply_sock(client->socket_fd, 300, MSG_INVALID_COMMAND);
        return;
    }

    
    
    // Check if username already exists
    if (find_account(username) != NULL) {
        send_reply_sock(client->socket_fd, 400, "User exists");
        return ; // Username already exists
    }
    
    
    // Generate new user_id (increment from highest existing ID)
    int new_user_id = 1;
    for (int i = 0; i < account_count; i++) {
        if (accounts[i].user_id >= new_user_id) {
            new_user_id = accounts[i].user_id + 1;
        }
    }
    
    // Add to in-memory array
    accounts[account_count].user_id = new_user_id;
    strncpy(accounts[account_count].username, username, MAX_USERNAME - 1);
    accounts[account_count].username[MAX_USERNAME - 1] = '\0';
    strncpy(accounts[account_count].password, password, MAX_PASSWORD - 1);
    accounts[account_count].password[MAX_PASSWORD - 1] = '\0';
    accounts[account_count].status = 1; // Default status: active
    
    // Append to file
    FILE *file = fopen(ACCOUNT_FILE_PATH, "a");
    if (file == NULL) {
        // Remove from array if file write fails
        account_count--;
        return ;
    }
    
    // Write in format: user_id|username|password|status
    fprintf(file, "%d|%s|%s|%d\n", 
            accounts[account_count].user_id,
            accounts[account_count].username,
            accounts[account_count].password,
            accounts[account_count].status);
    
    fclose(file);
    account_count++;
    send_reply_sock(client->socket_fd, 130, "Successfully Register");
    return;
    

    
}





/**
 * Handle message from client and route to appropriate handler
 * @param client_index Client index in clients array
 * @param message Message received from client
 */
void handle_message(int client_index, const char *message) {
    char command[BUFFER_SIZE];
    
    
    if (sscanf(message, "%s", command) != 1) {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_COMMAND);
        return;
    }
    
    if (strcmp(command, "LOGIN") == 0) {
        handle_login(client_index, message);
    } else if (strcmp(command, "LOGOUT") == 0) {
        handle_bye(client_index);
    } else if (strcmp(command, "REGISTER") == 0){
        handle_register(client_index, message);
    }
    else {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_COMMAND);
    }
}



/**
 * Main server function
 * Sets up TCP server, loads accounts, and handles client connections using select()
 * @param argc Number of command line arguments
 * @param argv Command line arguments (expects port number)
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Port_Number>\n", argv[0]);
        exit(1);
    }
    
    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }
    
    // Load account
    if (load_accounts(ACCOUNT_FILE_PATH) < 0) {
        fprintf(stderr, "Failed to load accounts\n");
        exit(1);
    }
    printf("Loaded %d accounts\n", account_count);
    
    
    // Step 1: Construct TCP Socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Set socket to reuse address
    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(1);
    }
    
    // Step 2: Bind Address to Socket
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(1);
    }
    
    // Step 3: Listen for Client Requests
    if (listen(listenfd, BACKLOG) < 0) {
        perror("Listen failed");
        exit(1);
    }
    printf("Server listening on port %d\n", port);
    

    // Initialize for select()
    int i, maxi, maxfd, connfd, sockfd;
    int nready;
    fd_set readfds, allset;
    
    maxfd = listenfd;
    maxi = -1;
    
    // Initialize client array
    for (i = 0; i < FD_SETSIZE; i++) {
        clients[i].socket_fd = -1;
        clients[i].user_id = -1;
        clients[i].is_logged_in = 0;
    }
    
    FD_ZERO(&allset); // clear all bits in fdset
    FD_SET(listenfd, &allset); // set the bit for the listenfd
    
    // Step 4: Communicate with Clients
    while (1) {
        readfds = allset;
        nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
        
        if (nready < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Select error");
            continue;
        }
        
        // Handle new connections
        if (FD_ISSET(listenfd, &readfds)) {
            socklen_t clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
            
            if (connfd < 0) {
                perror("Accept failed");
                continue;
            }
            
            printf("New client connected: %s:%d\n", 
                   inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
            
            // Find available slot in client array
            for (i = 0; i < FD_SETSIZE; i++) {
                if (clients[i].socket_fd < 0) {
                    clients[i].socket_fd = connfd;
                    clients[i].address = cliaddr;
                    clients[i].user_id = -1;
                    clients[i].is_logged_in = 0;

                    
                    break;
                }
            }
            
            if (i == FD_SETSIZE) {
                printf("Too many clients\n");
                close(connfd);
            } else {
                FD_SET(connfd, &allset); // turn on the bit for fd in fdset
                if (connfd > maxfd) {
                    maxfd = connfd; // update the maxfd for select()
                }
                if (i > maxi) {
                    maxi = i;
                }
                
                // Send connection success code
                send_reply_sock(connfd, 100, "Connection established");
                
                if (--nready <= 0) {
                    continue;
                }
            }
        }
        
        // Handle data from existing clients
        for (i = 0; i <= maxi; i++) {
            if ((sockfd = clients[i].socket_fd) < 0) {
                continue;
            }
            
            if (FD_ISSET(sockfd, &readfds)) {
                char message[BUFFER_SIZE];
                int ret = receive_line(sockfd, message, BUFFER_SIZE);
                
                if (ret <= 0) {
                    // Client disconnected or error
                    if (ret == 0) {
                        printf("Client disconnected: socket_fd=%d, user_id=%d\n", 
                               clients[i].socket_fd, clients[i].user_id);
                    }
                    close(sockfd);
                    FD_CLR(sockfd, &allset); // turn off the bit for fd in fdset
                    clients[i].socket_fd = -1;
                    clients[i].user_id = -1;
                    clients[i].is_logged_in = 0;
                    
                } else {
                    // Process message
                    printf("Received from client %d: %s\n", i, message);
                    // handle message 
                    handle_message(i, message);

                }
                
                if (--nready <= 0) {
                    break;
                }
            }
        }
    }
    
    close(listenfd);
    return 0;
}
