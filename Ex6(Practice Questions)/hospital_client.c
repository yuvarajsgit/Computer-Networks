#include <stdio.h>
#include <string.h>

#define PORT 8080

int main()
{
    SOCKET clientSocket;
    struct sockaddr_in server;
    int serverSize;

    char buffer[100];
    char response[500];

    int choice;
    int id;

    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    serverSize = sizeof(server);

    while(1)
    {
        printf("\n--- Hospital Appointment System ---\n");
        printf("1. Search Doctor\n");
        printf("2. Check Availability\n");
        printf("3. Book Appointment\n");
        printf("4. Cancel Appointment\n");
        printf("5. Exit\n");

        printf("Enter choice: ");
        scanf("%d", &choice);

        if(choice == 5)
            break;

        sprintf(buffer, "%d", choice);

        sendto(clientSocket, buffer, strlen(buffer) + 1, 0,
               (struct sockaddr *)&server, serverSize);

        if(choice == 2 || choice == 3 || choice == 4)
        {
            printf("Enter Doctor ID: ");
            scanf("%d", &id);

            sprintf(buffer, "%d", id);

            sendto(clientSocket, buffer, strlen(buffer) + 1, 0,
                   (struct sockaddr *)&server, serverSize);
        }

        recvfrom(clientSocket, response, sizeof(response), 0,
                 (struct sockaddr *)&server, &serverSize);

        printf("Server: %s\n", response);
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
