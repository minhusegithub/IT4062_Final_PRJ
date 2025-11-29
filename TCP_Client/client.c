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
    
    
    while (1) {
       /*
       
           TO DO

       */
    }
    
    close(sock_fd);
    return 0;
}

