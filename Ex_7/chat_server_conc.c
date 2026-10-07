#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(clientAddr);

    // 1. Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 2. Bind socket
    if (bind(server_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("Bind failed\n");
        close(server_fd);
        return 1;
    }

    // 3. Listen for connections
    if (listen(server_fd, 5) < 0)
    {
        printf("Listen failed\n");
        close(server_fd);
        return 1;
    }

    printf("TCP Concurrent Chat Server is running... Waiting for connections...\n");

    while (1)
    {
        // 4. Accept client connection
        client_fd = accept(server_fd, (struct sockaddr *)&clientAddr, &addrLen);
        if (client_fd < 0)
        {
            printf("Accept failed\n");
            continue;
        }

        // 5. Fork process to handle client concurrently[cite: 3]
        if (fork() == 0)
        {
            close(server_fd); // Child doesn't need listening socket[cite: 3]
            char buffer[1024], message[1024];

            printf("\nClient connected.\n");

            while (1)
            {
                int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
                if (n <= 0)
                {
                    printf("Client has disconnected.\n");
                    break;
                }
                buffer[n] = '\0';
                
                if (strcmp(buffer, "exit") == 0)
                {
                    printf("Client has disconnected.\n");
                    break;
                }

                printf("Client: %s\n", buffer);

                printf("Server reply: ");
                fgets(message, sizeof(message), stdin);
                message[strcspn(message, "\n")] = '\0';

                send(client_fd, message, strlen(message), 0);

                if (strcmp(message, "exit") == 0)
                {
                    printf("Chat ended by server.\n");
                    break;
                }
            }

            close(client_fd);
            exit(0);
        }

        close(client_fd); // Parent closes connected socket and waits for next client[cite: 3]
    }

    close(server_fd);
    return 0;
}