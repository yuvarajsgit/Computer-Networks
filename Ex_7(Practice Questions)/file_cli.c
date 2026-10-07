#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9001
#define MAX 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;

    char choice[10];
    char filename[100];
    char buffer[MAX];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&server_addr,
            sizeof(server_addr));

    printf("1. View files\n");
    printf("2. Download file\n");
    printf("Enter choice: ");

    fgets(choice, sizeof(choice), stdin);

    send(sock, choice, strlen(choice), 0);

    if (choice[0] == '1')
    {
        int n;

        n = recv(sock, buffer, MAX - 1, 0);

        buffer[n] = '\0';

        printf("\n%s", buffer);
    }

    else if (choice[0] == '2')
    {
        printf("Enter file name: ");

        fgets(filename, sizeof(filename), stdin);

        send(sock, filename, strlen(filename), 0);

        char save_name[150];

        sprintf(save_name, "downloaded_%s", filename);

        save_name[strcspn(save_name, "\n")] = '\0';

        FILE *fp = fopen(save_name, "wb");

        if (fp == NULL)
        {
            printf("Cannot create file.\n");
            close(sock);
            return 0;
        }

        int n;

        while ((n = recv(sock, buffer, MAX, 0)) > 0)
        {
            fwrite(buffer, 1, n, fp);
        }

        fclose(fp);

        printf("File downloaded successfully.\n");
    }

    close(sock);

    return 0;
}
