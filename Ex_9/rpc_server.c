#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define PORT 5000
#define BUFFER_SIZE 1024

// Remote procedures
int add(int a, int b)
{
    return a + b;
}

int subtract(int a, int b)
{
    return a - b;
}

int multiply(int a, int b)
{
    return a * b;
}

// Send response to client
void sendResponse(int serverSocket,
                  struct sockaddr_in *clientAddr,
                  socklen_t clientLen,
                  const char *response)
{
    sendto(serverSocket,
           response,
           strlen(response),
           0,
           (struct sockaddr *)clientAddr,
           clientLen);
}

int main()
{
    int serverSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    socklen_t clientLen;

    char buffer[BUFFER_SIZE];

    // Create UDP socket
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    if (serverSocket < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }

    // Server address
    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind socket
    if (bind(serverSocket,
             (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        perror("Bind failed");
        close(serverSocket);
        exit(1);
    }

    printf("========================================\n");
    printf("   Concurrent UDP RPC Server\n");
    printf("========================================\n");
    printf("Server started on port %d\n", PORT);
    printf("Waiting for client requests...\n");

    while (1)
    {
        int n;

        clientLen = sizeof(clientAddr);

        // Receive request from client
        n = recvfrom(serverSocket,
                     buffer,
                     BUFFER_SIZE - 1,
                     0,
                     (struct sockaddr *)&clientAddr,
                     &clientLen);

        if (n < 0)
        {
            perror("recvfrom failed");
            continue;
        }

        buffer[n] = '\0';

        printf("\nRequest received: %s\n", buffer);

        // Create child process
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Fork failed");
            continue;
        }

        // Child process
        if (pid == 0)
        {
            char operation[20];
            char extra[20];
            char response[BUFFER_SIZE];

            int a;
            int b;

            /*
             * Strictly accept only:
             *
             * ADD integer integer
             * SUB integer integer
             * MUL integer integer
             *
             * Example:
             * ADD 10 20
             * SUB 50 20
             * MUL 5 6
             */

            /*
             * First check that exactly THREE tokens
             * are present.
             *
             * If there are extra tokens, sscanf returns 4.
             */
            int count = sscanf(buffer,
                               "%19s %d %d %19s",
                               operation,
                               &a,
                               &b,
                               extra);

            if (count != 3)
            {
                strcpy(response,
                       "Invalid input. String can't be given as input");

                sendResponse(serverSocket,
                             &clientAddr,
                             clientLen,
                             response);

                printf("Rejected invalid input: %s\n", buffer);

                exit(0);
            }

            /*
             * Check whether the procedure name
             * is valid.
             */
            if (strcmp(operation, "ADD") != 0 &&
                strcmp(operation, "SUB") != 0 &&
                strcmp(operation, "MUL") != 0)
            {
                strcpy(response,
                       "Invalid procedure. Use ADD, SUB, or MUL");

                sendResponse(serverSocket,
                             &clientAddr,
                             clientLen,
                             response);

                printf("Rejected invalid procedure: %s\n", operation);

                exit(0);
            }

            /*
             * Execute the requested remote procedure.
             */
            int result;

            if (strcmp(operation, "ADD") == 0)
            {
                result = add(a, b);
            }
            else if (strcmp(operation, "SUB") == 0)
            {
                result = subtract(a, b);
            }
            else
            {
                result = multiply(a, b);
            }

            /*
             * Prepare result.
             */
            snprintf(response,
                     BUFFER_SIZE,
                     "Result of %s(%d, %d) = %d",
                     operation,
                     a,
                     b,
                     result);

            // Send result to client
            sendResponse(serverSocket,
                         &clientAddr,
                         clientLen,
                         response);

            printf("Child Process ID: %d\n", getpid());
            printf("Procedure executed: %s\n", operation);
            printf("Result: %d\n", result);

            exit(0);
        }
    }

    close(serverSocket);

    return 0;
}