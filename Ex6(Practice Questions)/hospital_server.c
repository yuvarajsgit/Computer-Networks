#include <stdio.h>
#include <string.h>

#define PORT 8080

struct Doctor
{
    int id;
    char name[30];
    char specialization[30];
    char slot[20];
    int available;
};

struct Doctor doctors[4] =
{
    {1, "Dr.Kumar", "Cardiologist", "10:00 AM", 1},
    {2, "Dr.Priya", "Dermatologist", "11:00 AM", 1},
    {3, "Dr.Arun", "Dentist", "12:00 PM", 1},
    {4, "Dr.Meena", "Physician", "2:00 PM", 1}
};

int main()
{
    SOCKET serverSocket;
    struct sockaddr_in server, client;
    int clientSize;

    char buffer[100];
    char response[500];

    int i, id;

    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr *)&server, sizeof(server));

    printf("Hospital Server Started...\n");

    clientSize = sizeof(client);

    while(1)
    {
        recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                 (struct sockaddr *)&client, &clientSize);

        if(strcmp(buffer, "1") == 0)
        {
            strcpy(response, "");

            for(i = 0; i < 4; i++)
            {
                if(doctors[i].available == 1)
                {
                    char temp[100];

                    sprintf(temp, "ID: %d  %s  %s  %s\n",
                            doctors[i].id,
                            doctors[i].name,
                            doctors[i].specialization,
                            doctors[i].slot);

                    strcat(response, temp);
                }
            }
        }

        else if(strcmp(buffer, "2") == 0)
        {
            recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&client, &clientSize);

            id = atoi(buffer);

            if(id >= 1 && id <= 4)
            {
                if(doctors[id-1].available == 1)
                    sprintf(response, "Doctor is available. Slot: %s",
                            doctors[id-1].slot);
                else
                    strcpy(response, "Doctor is not available.");
            }
            else
            {
                strcpy(response, "Invalid Doctor ID.");
            }
        }

        else if(strcmp(buffer, "3") == 0)
        {
            recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&client, &clientSize);

            id = atoi(buffer);

            if(id >= 1 && id <= 4)
            {
                if(doctors[id-1].available == 1)
                {
                    doctors[id-1].available = 0;
                    strcpy(response, "Appointment booked successfully.");
                }
                else
                {
                    strcpy(response, "Appointment slot is not available.");
                }
            }
            else
            {
                strcpy(response, "Invalid Doctor ID.");
            }
        }

        else if(strcmp(buffer, "4") == 0)
        {
            recvfrom(serverSocket, buffer, sizeof(buffer), 0,
                     (struct sockaddr *)&client, &clientSize);

            id = atoi(buffer);

            if(id >= 1 && id <= 4)
            {
                doctors[id-1].available = 1;
                strcpy(response, "Appointment cancelled successfully.");
            }
            else
            {
                strcpy(response, "Invalid Doctor ID.");
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
