#include "common.h"
#include "account.h"
#include "location.h"
#include "friend_request.h"
#include "friend.h"
#include "favorite.h"
#include "shared_location.h"

#define BACKLOG 20

// list of clients - using array with FD_SETSIZE
Client clients[FD_SETSIZE];

/**
 * Remove trailing \r and \n characters from string
 * @param str String to process (will be modified)
 */
void trim_CRLF(char *str)
{
    if (str == NULL)
        return;

    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r'))
    {
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
int receive_line(int sockfd, char *buf, size_t bufsz)
{
    size_t idx = 0;
    while (idx < bufsz - 1)
    {
        ssize_t n = recv(sockfd, &buf[idx], 1, 0);
        if (n == 1)
        {
            if (buf[idx - 1] == '\r' && buf[idx] == '\n')
            {
                buf[idx] = '\0';
                trim_CRLF(buf);
                return (int)strlen(buf);
            }
            idx++;
        }
        else if (n == 0)
        {
            return 0; /* peer closed */
        }
        else
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
    }
    buf[bufsz - 1] = '\0';
    trim_CRLF(buf);
    return (int)strlen(buf);
}

/**
 * Send reply to client with format "code message\r\n"
 * @param sockfd Client socket descriptor
 * @param code Response code (integer)
 * @param msg Message to send
 */
void send_reply_sock(int sockfd, int code, const char *msg)
{
    char out[MAX_LINE];
    int n = snprintf(out, sizeof(out), "%d %s\r\n", code, msg);
    if (n <= 0)
        return;

    size_t total = (size_t)n; // total length of the message
    size_t sent = 0;          // number of bytes sent

    while (sent < total)
    {
        ssize_t bytes_written = send(sockfd, out + sent, total - sent, 0);
        if (bytes_written > 0)
        {
            sent += (size_t)bytes_written;
        }
        else if (bytes_written < 0 && errno == EINTR)
        {
            continue; // if the send is interrupted, continue
        }
        else
        {
            break;
        }
    }
}

/**
 * Handle message from client and route to appropriate handler
 * @param client_index Client index in clients array
 * @param message Message received from client
 */
void handle_message(int client_index, const char *message)
{
    char command[BUFFER_SIZE];
    char args[BUFFER_SIZE]; // Biến chứa phần tham số phía sau lệnh (vd message = "ADD_LOCATION A|B|C" -> args = "A|B|C" )

    memset(args, 0, sizeof(args));
    memset(command, 0, sizeof(command));

    // Phân tích message thành 2 phần: Command và Arguments
    // %[^\n]: Đọc toàn bộ phần còn lại của dòng (tham số)
    int parsed = sscanf(message, "%s %[^\n]", command, args);

    if (parsed < 1)
    {
        send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_COMMAND);
        return;
    }

    // Code cũ
    // if (sscanf(message, "%s", command) != 1) {
    //     send_reply_sock(clients[client_index].socket_fd, 300, MSG_INVALID_COMMAND);
    //     return;
    // }

    if (strcmp(command, REQ_LOGIN) == 0)
    {
        handle_login(client_index, args);
    }
    else if (strcmp(command, REQ_LOGOUT) == 0)
    {
        handle_logout(client_index, args);
    }
    else if (strcmp(command, REQ_REGISTER) == 0)
    {
        handle_register(client_index, args);
    }
    else if (strcmp(command, REQ_ADD_LOCATION) == 0)
    {
        // args bao gồm "name|addr|cat|desc"
        handle_add_location(client_index, args);
    }
    else if (strcmp(command, REQ_UPDATE_LOCATION) == 0)
    {
        handle_update_location(client_index, args);
    }
    else if (strcmp(command, REQ_DELETE_LOCATION) == 0)
    {
        handle_delete_location(client_index, args);
    }
    else if (strcmp(command, REQ_GET_LOCATIONS) == 0)
    {
        // args chứa category hoặc rỗng
        handle_get_locations(client_index, args);
    }
    else if (strcmp(command, REQ_VIEW_MY_LOCATIONS) == 0)
    {
        handle_view_my_locations(client_index);
    }
    else if (strcmp(command, REQ_SEND_FRIEND_REQUEST) == 0)
    {
        // args chứa username
        handle_send_friend_request(client_index, args);
    }
    else if (strcmp(command, REQ_GET_FRIEND_REQUESTS) == 0)
    {
        handle_get_friend_requests(client_index);
    }
    else if (strcmp(command, REQ_ACCEPT_FRIEND_REQUEST) == 0)
    {
        handle_accept_friend_request(client_index, args);
    }
    else if (strcmp(command, REQ_REJECT_FRIEND_REQUEST) == 0)
    {
        handle_reject_friend_request(client_index, args);
    }
    else if (strcmp(command, REQ_GET_FRIENDS) == 0)
    {
        handle_get_friends(client_index);
    }
    else if (strcmp(command, REQ_SAVE_TO_FAV_LOCATION) == 0)
    {
        // args chứa ID địa điểm
        handle_save_favorite(client_index, args);
    }
    else if (strcmp(command, REQ_VIEW_FAVORITE_LOCATIONS) == 0)
    {
        handle_view_favorite_locations(client_index);
    }
    else if (strcmp(command, REQ_UNFRIEND) == 0)
    {
        handle_unfriend(client_index, args);
    }
    else if (strcmp(command, REQ_SHARE_LOCATION) == 0)
    {
        handle_share_location(client_index, args);
    }
    else if (strcmp(command, REQ_GET_SHARED_LOCATIONS) == 0)
    {
        handle_get_shared_locations(client_index);
    }
    else
    {
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
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <Port_Number>\n", argv[0]);
        exit(1);
    }

    int port = atoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }

    // Load account
    if (load_accounts(ACCOUNT_FILE_PATH) < 0)
    {
        fprintf(stderr, "Failed to load accounts\n");
        exit(1);
    }
    printf("Loaded %d accounts\n", account_count);

    // Load locations
    if (load_locations(LOCATION_FILE_PATH) < 0)
    {
        fprintf(stderr, "Warning: Failed to load locations or file empty\n");
    }
    printf("Loaded %d locations\n", location_count);

    // Load friend requests
    if (load_friend_requests(FRIEND_REQUEST_FILE_PATH) < 0)
    {
        fprintf(stderr, "Warning: Failed to load friend requests or file empty\n");
    }
    printf("Loaded %d friend_requests\n", friend_request_count);

    // Load friends
    if (load_friends(FRIEND_FILE_PATH) < 0)
    {
        fprintf(stderr, "Warning: Failed to load friends or file empty\n");
    }
    printf("Loaded %d friends \n", friend_count);

    // Load favorites
    if (load_favorites(FAVORITE_FILE_PATH) < 0)
    {
        fprintf(stderr, "Warning: Failed to load favorites\n");
    }
    printf("Loaded favorites for %d users\n", favorite_count);

    // Load shared locations
    if (load_shared_locations(SHARED_FILE_PATH) < 0)
    {
        fprintf(stderr, "Warning: Failed to load shared locations\n");
    }
    printf("Loaded shared lists for %d users\n", shared_count);

    // Step 1: Construct TCP Socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Set socket to reuse address
    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt failed");
        exit(1);
    }

    // Step 2: Bind Address to Socket
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Bind failed");
        exit(1);
    }

    // Step 3: Listen for Client Requests
    if (listen(listenfd, BACKLOG) < 0)
    {
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
    for (i = 0; i < FD_SETSIZE; i++)
    {
        clients[i].socket_fd = -1;
        clients[i].user_id = -1;
        clients[i].is_logged_in = 0;
    }

    FD_ZERO(&allset);          // clear all bits in fdset
    FD_SET(listenfd, &allset); // set the bit for the listenfd

    // Step 4: Communicate with Clients
    while (1)
    {
        readfds = allset;
        nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);

        if (nready < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("Select error");
            continue;
        }

        // Handle new connections
        if (FD_ISSET(listenfd, &readfds))
        {
            socklen_t clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);

            if (connfd < 0)
            {
                perror("Accept failed");
                continue;
            }

            printf("New client connected: %s:%d\n",
                   inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

            // Find available slot in client array
            for (i = 0; i < FD_SETSIZE; i++)
            {
                if (clients[i].socket_fd < 0)
                {
                    clients[i].socket_fd = connfd;
                    clients[i].address = cliaddr;
                    clients[i].user_id = -1;
                    clients[i].is_logged_in = 0;

                    break;
                }
            }

            if (i == FD_SETSIZE)
            {
                printf("Too many clients\n");
                close(connfd);
            }
            else
            {
                FD_SET(connfd, &allset); // turn on the bit for fd in fdset
                if (connfd > maxfd)
                {
                    maxfd = connfd; // update the maxfd for select()
                }
                if (i > maxi)
                {
                    maxi = i;
                }

                // Send connection success code
                send_reply_sock(connfd, 100, "Connection established");

                if (--nready <= 0)
                {
                    continue;
                }
            }
        }

        // Handle data from existing clients
        for (i = 0; i <= maxi; i++)
        {
            if ((sockfd = clients[i].socket_fd) < 0)
            {
                continue;
            }

            if (FD_ISSET(sockfd, &readfds))
            {
                char message[BUFFER_SIZE];
                int ret = receive_line(sockfd, message, BUFFER_SIZE);

                if (ret <= 0)
                {
                    // Client disconnected or error
                    if (ret == 0)
                    {
                        printf("Client disconnected: socket_fd=%d, user_id=%d\n",
                               clients[i].socket_fd, clients[i].user_id);
                    }
                    close(sockfd);
                    FD_CLR(sockfd, &allset); // turn off the bit for fd in fdset
                    clients[i].socket_fd = -1;
                    clients[i].user_id = -1;
                    clients[i].is_logged_in = 0;
                }
                else
                {
                    // Process message
                    printf("Received from client %d: %s\n", i, message);
                    // handle message
                    handle_message(i, message);
                }

                if (--nready <= 0)
                {
                    break;
                }
            }
        }
    }

    close(listenfd);
    return 0;
}
