#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024];
    char input_str[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation error\n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return 1;
    }

    printf("Connected to Palindrome Server.\n");
    printf("Enter a string to check for palindrome: ");
    fgets(input_str, sizeof(input_str), stdin);

    // Send string to server
    write(sock, input_str, strlen(input_str));

    // Read response from server
    memset(buffer, 0, sizeof(buffer));
    int n = read(sock, buffer, sizeof(buffer) - 1);
    if (n > 0) {
        printf("Server Response: %s\n", buffer);
    }

    close(sock);
    return 0;
}