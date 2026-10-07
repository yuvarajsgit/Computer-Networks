#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>

#define PORT 9001
#define MAX 1024

void *client_handler(void *arg)
{
    int client = *(int *)arg;
    free(arg);

    char choice[10];
    char filename[100];
    char buffer[MAX];

    recv(client, choice, sizeof(choice), 0);

    if (choice[0] == '1')
    {
        DIR *folder;
        struct dirent *file;

        folder = opendir("shared");

        if (folder == NULL)
        {
            send(client, "Shared folder not found.\n", 25, 0);
        }
        else
        {
            strcpy(buffer, "Available files:\n");

            while ((file = readdir(folder)) != NULL)
            {
                if (file->d_name[0] != '.')
                {
                    strcat(buffer, file->d_name);
                    strcat(buffer, "\n");
                }
            }

            closedir(folder);

            send(client, buffer, strlen(buffer), 0);
        }
    }

    else if (choice[0] == '2')
    {
        recv(client, filename, sizeof(filename), 0);

        filename[strcspn(filename, "\n")] = '\0';

        char path[200];

        sprintf(path, "shared/%s", filename);

        FILE *fp = fopen(path, "rb");

        if (fp == NULL)
        {
            send(client, "File not found.\n", 16, 0);
        }
        else
        {
            int n;

            while ((n = fread(buffer, 1, MAX, fp)) > 0)
            {
                send(client, buffer, n, 0);
            }

            fclose(fp);
        }
    }

    close(client);

    return NULL;
}

int main()
{
    int server, client;
    struct sockaddr_in server_addr, client_addr;
    socklen_t size;

    server = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server, (struct sockaddr *)&server_addr,
         sizeof(server_addr));

    listen(server, 5);

    printf("File Sharing Server running...\n");
    printf("Waiting for clients...\n");

    while (1)
    {
        size = sizeof(client_addr);

        client = accept(server,
                        (struct sockaddr *)&client_addr,
                        &size);

        printf("Client connected.\n");

        int *p = malloc(sizeof(int));
        *p = client;

        pthread_t thread;

        pthread_create(&thread, NULL,
                       client_handler, p);

        pthread_detach(thread);
    }

    close(server);

    return 0;
}
