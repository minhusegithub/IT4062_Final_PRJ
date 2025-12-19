#include "common.h"
#include "account.h"
#include "location.h"
#include "friend_request.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

/**
 * Main client function
 * Connects to server, handles user menu interactions, and manages login state
 * @param argc Number of command line arguments
 * @param argv Command line arguments (expects IP address and port number)
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <IP_Addr> <Port_Number>\n", argv[0]);
        exit(1);
    }

    char *ip_addr = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }

    //  socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Construct server
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip_addr, &server_addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid IP address\n");
        exit(1);
    }

    // Kết nối đến server
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        exit(1);
    }

    printf("Connected to server %s:%d\n", ip_addr, port);

    char line[MAX_LINE];
    /* read welcome */
    int r = receive_line(sock_fd, line, sizeof(line));
    if (r <= 0)
    {
        fprintf(stderr, "Server closed or error\n");
        close(sock_fd);
        return 1;
    }
    printf("Server send: %s\n", line);

    // Vòng lặp chính để xử lý lệnh người dùng
    int logged_in = 0;

    while (1)
    {
        printf("----------------MENU----------------\n");
        printf("1. Login\n");
        printf("2. Register\n");
        printf("3. Logout\n");
        printf("4. Add Location\n");
        printf("5. View Locations\n");
        printf("6. View My Locations\n");
        printf("7. Update Location\n");
        printf("8. Delete Location\n");
        printf("9. Send Friend Request\n");
        printf("10. Exit\n");
        printf("Your choice (1-10): ");
        if (!fgets(line, sizeof(line), stdin))
            break;
        int choice = atoi(line);

        if (choice == 1)
        {
            if (do_login(sock_fd) == 1)
                logged_in = 1;
        }
        else if (choice == 2)
        {
            do_register(sock_fd);
        }
        else if (choice == 3)
        {
            if (do_logout(sock_fd) == 1)
                logged_in = 0;
        }
        else if (choice == 4)
        {
            do_add_location(sock_fd);
        }
        else if (choice == 5)
        {
            do_get_locations(sock_fd);
        }
        else if (choice == 6) {         
            do_view_my_locations(sock_fd);
        }
        else if (choice == 7) 
        {
            do_update_location(sock_fd);
        }
        else if (choice == 8) 
        {
            do_delete_location(sock_fd);
        }
        else if (choice == 9)
        {
            do_send_friend_request(sock_fd);
        }
        else if (choice == 10)
        {
            if (logged_in)
                do_logout(sock_fd);
            printf("Goodbye!\n");
            break;
        }
        else
            printf("Invalid choice. Please enter 1-7.\n");
    }

    close(sock_fd);
    return 0;
}
