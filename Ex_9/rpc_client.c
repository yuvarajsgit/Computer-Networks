#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 5000
#define BUFFER_SIZE 1024

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddr;
    socklen_t serverLen;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Create UDP socket
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (clientSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    serverLen = sizeof(serverAddr);

    printf("UDP RPC Client\n");

    while (1)
    {
        printf("\nEnter RPC request:\n");
        printf("ADD a b\n");
        printf("SUB a b\n");
        printf("MUL a b\n");
        printf("Enter request: ");

        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0)
        {
            break;
        }

        // Send request to server
        sendto(clientSocket,
               buffer,
               strlen(buffer),
               0,
               (struct sockaddr *)&serverAddr,
               serverLen);

        // Receive result
        int n = recvfrom(clientSocket,
                         response,
                         BUFFER_SIZE - 1,
                         0,
                         (struct sockaddr *)&serverAddr,
                         &serverLen);

        if (n < 0)
        {
            perror("recvfrom failed");
            continue;
        }

        response[n] = '\0';

        printf("Server Response: %s\n", response);
    }

    close(clientSocket);

    return 0;
}