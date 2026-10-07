#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 9000
#define MAX 1024

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[MAX];
    char username[50];
    char password[50];
    char answer[10];
    int i;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect(sock, (struct sockaddr *)&server_addr,
            sizeof(server_addr));

    recv(sock, buffer, MAX, 0);
    printf("%s", buffer);

    fgets(username, sizeof(username), stdin);
    send(sock, username, strlen(username), 0);

    recv(sock, buffer, MAX, 0);
    printf("%s", buffer);

    fgets(password, sizeof(password), stdin);
    send(sock, password, strlen(password), 0);

    recv(sock, buffer, MAX, 0);
    printf("%s", buffer);

    if (strstr(buffer, "failed") != NULL)
    {
        close(sock);
        return 0;
    }

    for (i = 0; i < 3; i++)
    {
        recv(sock, buffer, MAX, 0);
        printf("%s", buffer);
    }

    recv(sock, buffer, MAX, 0);
    printf("%s", buffer);

    for (i = 0; i < 3; i++)
    {
        printf("Answer %d: ", i + 1);

        fgets(answer, sizeof(answer), stdin);

        send(sock, answer, strlen(answer), 0);
    }

    recv(sock, buffer, MAX, 0);

    printf("%s", buffer);

    close(sock);

    return 0;
}
