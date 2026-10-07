#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int sockfd;
    char message[1024], buffer[1024];
    struct sockaddr_in serverAddr;

    // 1. Create TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // 2. Connect to TCP server
    if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Connection failed\n");
        close(sockfd);
        return 1;
    }

    printf("TCP Chat Client connected. Type 'exit' to quit.\n");

    while (1)
    {
        printf("Client: ");
        fgets(message, sizeof(message), stdin);
        message[strcspn(message, "\n")] = '\0';

        // Send message using TCP send
        send(sockfd, message, strlen(message), 0);

        if (strcmp(message, "exit") == 0)
        {
            break;
        }

        // Receive reply using TCP recv
        int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0)
        {
            printf("Server has disconnected.\n");
            break;
        }
        buffer[n] = '\0';

        if (strcmp(buffer, "exit") == 0)
        {
            printf("Server has ended the chat.\n");
            break;
        }

        printf("Server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}