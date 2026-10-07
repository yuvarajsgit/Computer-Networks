#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SIZE 20
#define PORT 5600

struct ARP
{
    char ip[20];
    char mac[20];
};

int hash(char ip[])
{
    int sum = 0, i;

    for (i = 0; ip[i] != '\0'; i++)
        sum += ip[i];

    return sum % SIZE;
}

int main()
{
    FILE *fp;
    char line[200], ip[20], mac[20];
    int idx;

    struct ARP table[SIZE];

    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len;
    int ch;

    /* Initialize ARP table */
    for (int i = 0; i < SIZE; i++)
    {
        table[i].ip[0] = '\0';
        table[i].mac[0] = '\0';
    }

    /*
       Add laptop's own IP and MAC address.
       Change these two values according to your laptop.
    */
    strcpy(ip, "192.168.1.10");
    strcpy(mac, "AA:BB:CC:DD:EE:FF");

    idx = hash(ip);

    strcpy(table[idx].ip, ip);
    strcpy(table[idx].mac, mac);

    printf("Own Laptop Entry Added:\n");
    printf("IP  : %s\n", ip);
    printf("MAC : %s\n", mac);

    /*
       Read other ARP entries from the system
    */
    fp = popen("ip neigh", "r");

    if (fp != NULL)
    {
        while (fgets(line, sizeof(line), fp))
        {
            if (strstr(line, "lladdr") != NULL)
            {
                sscanf(line, "%s", ip);

                sscanf(strstr(line, "lladdr") + 7, "%s", mac);

                /*
                   Do not overwrite our own laptop entry
                */
                if (strcmp(ip, table[idx].ip) != 0)
                {
                    int new_idx = hash(ip);

                    strcpy(table[new_idx].ip, ip);
                    strcpy(table[new_idx].mac, mac);
                }
            }
        }

        pclose(fp);
    }

    /* Create TCP socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    int opt = 1;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    /* Bind */
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    /* Listen */
    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("\nARP Server listening on port %d...\n", PORT);

    while (1)
    {
        addr_len = sizeof(client_addr);

        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &addr_len);

        if (client_fd < 0)
        {
            perror("Accept failed");
            continue;
        }

        printf("\nClient connected.\n");

        while (1)
        {
            if (recv(client_fd, &ch, sizeof(ch), 0) <= 0)
                break;

            /* Exit */
            if (ch == 3)
            {
                break;
            }

            /* Display ARP table */
            else if (ch == 2)
            {
                send(client_fd,
                     table,
                     sizeof(table),
                     0);
            }

            /* Search IP */
            else if (ch == 1)
            {
                if (recv(client_fd,
                         ip,
                         sizeof(ip),
                         0) <= 0)
                    break;

                int found = 0;

                /*
                   Search entire table instead of
                   depending only on the hash index.
                */
                for (int i = 0; i < SIZE; i++)
                {
                    if (strcmp(table[i].ip, ip) == 0)
                    {
                        found = 1;

                        send(client_fd,
                             &found,
                             sizeof(found),
                             0);

                        send(client_fd,
                             table[i].mac,
                             sizeof(table[i].mac),
                             0);

                        break;
                    }
                }

                /*
                   IP not found
                */
                if (found == 0)
                {
                    send(client_fd,
                         &found,
                         sizeof(found),
                         0);
                }
            }
        }

        close(client_fd);

        printf("Client disconnected.\n");
    }

    close(server_fd);

    return 0;
}