#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SIZE 20
#define PORT 5600

struct ARP
{
    char ip[20];
    char mac[20];
};

void display_table(struct ARP table[])
{
    printf("\n");
    printf("========================================\n");
    printf("          ARP TABLE FROM SERVER\n");
    printf("========================================\n");
    printf("%-20s %-20s\n", "IP Address", "MAC Address");
    printf("----------------------------------------\n");

    for (int i = 0; i < SIZE; i++)
    {
        if (strlen(table[i].ip) > 0)
        {
            printf("%-20s %-20s\n",
                   table[i].ip,
                   table[i].mac);
        }
    }

    printf("========================================\n");
}

int main()
{
    char ip[20];
    char mac[20];

    int sock_fd;
    int ch;

    struct sockaddr_in server_addr;
    struct ARP table[SIZE];

    /* Create TCP socket */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (sock_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;

    /*
       Server is running on same laptop
    */
    server_addr.sin_addr.s_addr =
        inet_addr("127.0.0.1");

    server_addr.sin_port = htons(PORT);

    /* Connect to server */
    if (connect(sock_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection to server failed");
        close(sock_fd);
        return 1;
    }

    printf("Connected to ARP Server.\n");

    while (1)
    {
        printf("\n");
        printf("1. Search MAC Address\n");
        printf("2. Display ARP Table\n");
        printf("3. Exit\n");
        printf("Enter choice: ");

        scanf("%d", &ch);

        send(sock_fd,
             &ch,
             sizeof(ch),
             0);

        /* Exit */
        if (ch == 3)
        {
            break;
        }

        /* Display ARP table */
        else if (ch == 2)
        {
            recv(sock_fd,
                 table,
                 sizeof(table),
                 0);

            display_table(table);
        }

        /* Search */
        else if (ch == 1)
        {
            printf("\nEnter IP address: ");
            scanf("%19s", ip);

            send(sock_fd,
                 ip,
                 sizeof(ip),
                 0);

            int found;

            recv(sock_fd,
                 &found,
                 sizeof(found),
                 0);

            if (found == 1)
            {
                recv(sock_fd,
                     mac,
                     sizeof(mac),
                     0);

                printf("\nMAC Address found: %s\n",
                       mac);
            }
            else
            {
                printf("\nMAC Address not found for IP: %s\n",
                       ip);
            }
        }

        else
        {
            printf("\nInvalid choice.\n");
        }
    }

    close(sock_fd);

    return 0;
}