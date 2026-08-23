#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// Function to check if a string is a palindrome
int is_palindrome(char str[]) {
    int l = 0;
    int h = strlen(str) - 1;

    // Remove trailing newline if present
    if (str[h] == '\n') {
        str[h] = '\0';
        h--;
    }

    while (h > l) {
        if (str[l++] != str[h--]) {
            return 0; // Not a palindrome
        }
    }
    return 1; // Palindrome
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("Palindrome TCP Server is running on port 8080...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        printf("\nConnected to client: IP = %s, Port = %d\n", client_ip, ntohs(client_addr.sin_port));

        // Read string from client
        memset(buffer, 0, sizeof(buffer));
        int n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            printf("Received string from client: %s", buffer);

            char response[100];
            if (is_palindrome(buffer)) {
                strcpy(response, "The string IS a palindrome.");
            } else {
                strcpy(response, "The string is NOT a palindrome.");
            }

            // Send result back to client
            write(client_fd, response, strlen(response));
        }

        close(client_fd);
        printf("Client session closed.\n");
    }

    close(server_fd);
    return 0;
}