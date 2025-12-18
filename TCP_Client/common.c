#include "common.h"


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
    { // read bytes from socket one by one until \n character
        ssize_t n = recv(sockfd, &buf[idx], 1, 0);
        if (n == 1)
        {
            if (buf[idx - 1] == '\r' && buf[idx] == '\n')
            {
                buf[idx - 1] = '\0';

                return (int)strlen(buf);
            }
            idx++;
        }
        else if (n == 0)
        {
            return 0;
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
 * Send reply to client with format "message\r\n"
 * @param sockfd Client socket descriptor
 * @param msg Message to send
 */
void send_to_server(int sockfd, const char *msg)
{
    char out[MAX_LINE];
    int n = snprintf(out, sizeof(out), "%s\r\n", msg);
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
 * Remove leading and trailing whitespace characters (space, \t, \r, \n) from a string
 * @param str Pointer to the null-terminated string to be trimmed (modified in-place)
 */
void trim(char *str) {
    if (str == NULL) return;

    // 1. Cắt khoảng trắng ở cuối (Right trim)
    size_t len = strlen(str);
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }
    str[len] = '\0'; // Đặt ký tự kết thúc chuỗi tại vị trí mới

    // 2. Cắt khoảng trắng ở đầu (Left trim)
    char *start = str;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (start != str) {
        char *dst = str; // Con trỏ đích (bắt đầu chuỗi)
        
        // Chạy vòng lặp copy từng ký tự
        while (*start != '\0') {
            *dst = *start;
            dst++;
            start++;
        }

        *dst = '\0'; 
    }
}