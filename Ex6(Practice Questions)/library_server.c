#include <stdio.h>
#include <string.h>

#define PORT 8080

struct Book
{
    int id;
    char title[30];
    char author[30];
    char category[20];
    int available;
};

struct Book books[5] =
{
    {1, "C Programming", "Dennis", "Programming", 1},
    {2, "Computer Networks", "Tanenbaum", "Networking", 1},
    {3, "Operating Systems", "Galvin", "OS", 1},
    {4, "Java", "Herbert", "Programming", 1},
    {5, "Data Structures", "Lipschutz", "DSA", 1}
};

int main()
{
    SOCKET serverSocket;
    struct sockaddr_in server, client;
    int clientSize;

    char buffer[100];
    char response[500];

    int choice;
    int id;
    int i;

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr *)&server, sizeof(server));

    printf("Library Server Started...\n");

    clientSize = sizeof(client);

    while(1)
    {
        recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                 (struct sockaddr *)&client, &clientSize);

        choice = atoi(buffer);

        if(choice == 1)
        {
            strcpy(response, "");

            for(i = 0; i < 5; i++)
            {
                char temp[100];

                sprintf(temp, "ID: %d  Title: %s  Author: %s  Category: %s\n",
                        books[i].id,
                        books[i].title,
                        books[i].author,
                        books[i].category);

                strcat(response, temp);
            }
        }

        else if(choice == 2)
        {
            recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&client, &clientSize);

            id = atoi(buffer);

            if(id >= 1 && id <= 5)
            {
                if(books[id-1].available == 1)
                    strcpy(response, "Book is available.");
                else
                    strcpy(response, "Book is not available.");
            }
            else
            {
                strcpy(response, "Invalid Book ID.");
            }
        }

        else if(choice == 3)
        {
            recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&client, &clientSize);

            id = atoi(buffer);

            if(id >= 1 && id <= 5)
            {
                if(books[id-1].available == 1)
                {
                    books[id-1].available = 0;
                    strcpy(response, "Book issued successfully.");
                }
                else
                {
                    strcpy(response, "Book is already issued.");
                }
            }
            else
            {
                strcpy(response, "Invalid Book ID.");
            }
        }

        else if(choice == 4)
        {
            recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&client, &clientSize);

            id = atoi(buffer);

            if(id >= 1 && id <= 5)
            {
                books[id-1].available = 1;
                strcpy(response, "Book returned successfully.");
            }
            else
            {
                strcpy(response, "Invalid Book ID.");
            }
        }

        else if(choice == 5)
        {
            strcpy(response, "");

            for(i = 0; i < 5; i++)
            {
                if(books[i].available == 0)
                {
                    char temp[100];

                    sprintf(temp, "ID: %d  Title: %s\n",
                            books[i].id,
                            books[i].title);

                    strcat(response, temp);
                }
            }
        }

        else
        {
            strcpy(response, "Invalid choice.");
        }

        sendto(serverSocket, response, strlen(response) + 1, 0,
               (struct sockaddr *)&client, clientSize);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
