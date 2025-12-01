#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUFFER_SIZE 1024
#define MAX_LINE 1024

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
    while (idx < bufsz - 1) { // read bytes from socket one by one until \n character 
        ssize_t n = recv(sockfd, &buf[idx], 1, 0);
        if (n == 1) {
            if (buf[idx] == '\n') {
                buf[idx] = '\0';
                trim_CRLF(buf);
                return (int)strlen(buf);
            }
            idx++;
        } else if (n == 0) {
            return 0; 
        } else {
            if (errno == EINTR) continue;
            return -1;
        }
    }
    buf[bufsz-1] = '\0';
    trim_CRLF(buf);
    return (int)strlen(buf);
}

int do_login(int sock) {
    char username[512], password[512], out[MAX_LINE], response[MAX_LINE];
    
    printf("Enter username: ");
    if (!fgets(username, sizeof(username), stdin)) return -1;
    printf("Enter password: ");
    if (!fgets(password, sizeof(password), stdin)) return -1;

    /* trim trailing CR/LF from stdin */
    trim_CRLF(username);
    trim_CRLF(password);
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("Username and password cannot be empty\n");
        return -1;
    }
    
    int n = snprintf(out, sizeof(out), "LOGIN %s %s\r\n", username ,password); // format the message to send to server
    if (n < 0 || (size_t)n >= sizeof(out)) {
        fprintf(stderr, "Input too long.\n");
        return -1;
    }
    
    if (send(sock, out, (size_t)n, 0) <= 0) {
        perror("send");
        return -1;
    }
    
    if (receive_line(sock, response, sizeof(response)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return -1;
    }
    
    printf("Server send: %s\n", response);
    return (atoi(response) == 110 );
}

void do_register(int sock){
    char username[512], password[512], out[MAX_LINE], response[MAX_LINE];
    
    printf("Enter username to register: ");
    if (!fgets(username, sizeof(username), stdin)) return;
    printf("Enter password to register: ");
    if (!fgets(password, sizeof(password), stdin)) return;

    /* trim trailing CR/LF from stdin */
    trim_CRLF(username);
    trim_CRLF(password);
    
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("Username and password cannot be empty\n");
        return;
    }
    
    int n = snprintf(out, sizeof(out), "REGISTER %s %s\r\n", username ,password); // format the message to send to server
    if (n < 0 || (size_t)n >= sizeof(out)) {
        fprintf(stderr, "Input too long.\n");
        return;
    }
    
    if (send(sock, out, (size_t)n, 0) <= 0) {
        perror("send");
        return;
    } // Đảm bảo send đủ bytes TODO //
    
    if (receive_line(sock, response, sizeof(response)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return;
    }
    
    printf("Server send: %s\n", response);
    return;
}

int do_logout(int sock) {
    char line[MAX_LINE];
    const char *out = "LOGOUT\r\n"; // format the message to send to server
    
    if (send(sock, out, strlen(out), 0) <= 0) {
        perror("send");
        return -1;
    }
    
    if (receive_line(sock, line, sizeof(line)) <= 0) {
        fprintf(stderr, "Server closed\n");
        return -1;
    }
    
    printf("%s\n", line);
    int code = atoi(line);

    return (code == 130) ? 1 : 0;
}


/**
 * Main client function
 * Connects to server, handles user menu interactions, and manages login state
 * @param argc Number of command line arguments
 * @param argv Command line arguments (expects IP address and port number)
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_Addr> <Port_Number>\n", argv[0]);
        exit(1);
    }
    
    char *ip_addr = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }
    
    //  socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(1);
    }
    
    // Construct server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip_addr, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid IP address\n");
        exit(1);
    }
    
    // Kết nối đến server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }
    
    printf("Connected to server %s:%d\n", ip_addr, port);
    
    char line[MAX_LINE];
    /* read welcome */
    int r = receive_line(sock_fd, line, sizeof(line));
    if (r <= 0) { 
        fprintf(stderr, "Server closed or error\n");
        close(sock_fd); 
        return 1;
    }
    printf("Server send: %s\n", line);
    
    // Vòng lặp chính để xử lý lệnh người dùng
    int logged_in = 0;
    
    while (1) {
        printf("----------------MENU----------------\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("3. Logout\n");
        printf("4. Exit\n");
        printf("Your choice(1-4): ");
        
        if (!fgets(line, sizeof(line), stdin)) break;
        int choice = atoi(line);

        if (choice == 1) {    
            if (do_login(sock_fd) == 1) logged_in = 1;
        } else if(choice == 2) {
            do_register(sock_fd);
        }else if (choice == 3) {
            if (do_logout(sock_fd) == 1) logged_in = 0;
        } else if (choice == 4) { 
            if (logged_in) do_logout(sock_fd); 
            printf("Goodbye!\n");
            break;
        } else printf("Invalid choice. Please enter 1, 2, 3, or 4.\n");
        
    }
    
    close(sock_fd);
    return 0;
}

