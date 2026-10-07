#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define TIMEOUT_SEC  2
#define MAX_RETRIES  3

int wait_for_reply(int s, struct sockaddr_in *server, socklen_t len,
                   char *msg, char *reply)
{
    struct timeval tv;
    tv.tv_sec  = TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    int n = recvfrom(s, reply, sizeof(reply), 0,
                     (struct sockaddr*)server, &len);
    if (n > 0)
    {
        reply[n] = '\0';
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int s;
    char msg[256], reply[2048];
    long txn_id = 0;

    if (argc < 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server;
    socklen_t len = sizeof(server);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[2]));
    server.sin_addr.s_addr = inet_addr(argv[1]);

    printf("=== Library Management Client ===\n");
    printf("Connected to %s:%s\n\n", argv[1], argv[2]);

    while (1)
    {
        printf("1. List available books\n");
        printf("2. Search book\n");
        printf("3. Issue book\n");
        printf("4. Return book\n");
        printf("5. Exit\n");
        printf("Choice: ");
        int choice;
        scanf("%d", &choice);
        while (getchar() != '\n');

        if (choice == 5)
        {
            sendto(s, "QUIT", 4, 0, (struct sockaddr*)&server, len);
            printf("Goodbye.\n");
            break;
        }

        char arg1[50] = "", arg2[50] = "";

        switch (choice)
        {
            case 1:
                break;
            case 2:
                printf("Enter book ID or title to search: ");
                fgets(arg1, sizeof(arg1), stdin);
                arg1[strcspn(arg1, "\n")] = '\0';
                break;
            case 3:
                printf("Enter book ID: ");
                fgets(arg1, sizeof(arg1), stdin);
                arg1[strcspn(arg1, "\n")] = '\0';
                printf("Enter member ID: ");
                fgets(arg2, sizeof(arg2), stdin);
                arg2[strcspn(arg2, "\n")] = '\0';
                break;
            case 4:
                printf("Enter book ID: ");
                fgets(arg1, sizeof(arg1), stdin);
                arg1[strcspn(arg1, "\n")] = '\0';
                printf("Enter member ID: ");
                fgets(arg2, sizeof(arg2), stdin);
                arg2[strcspn(arg2, "\n")] = '\0';
                break;
            default:
                printf("Invalid choice.\n\n");
                continue;
        }

        txn_id++;

        /* Build request: "TXN_ID|COMMAND|ARG1|ARG2" */
        if (choice == 1)
            sprintf(msg, "%ld|LIST||", txn_id);
        else if (choice == 2)
            sprintf(msg, "%ld|SEARCH|%s|", txn_id, arg1);
        else if (choice == 3)
            sprintf(msg, "%ld|ISSUE|%s|%s", txn_id, arg1, arg2);
        else
            sprintf(msg, "%ld|RETURN|%s|%s", txn_id, arg1, arg2);

        /* Send with retransmission */
        int success = 0;
        for (int i = 0; i < MAX_RETRIES; i++)
        {
            sendto(s, msg, strlen(msg), 0,
                   (struct sockaddr*)&server, len);

            len = sizeof(server);
            if (wait_for_reply(s, &server, len, msg, reply))
            {
                success = 1;
                break;
            }
            printf("[Retransmit %d/%d] Timeout...\n", i + 1, MAX_RETRIES);
        }

        if (success)
        {
            /* Strip "ACK|txn_id|" prefix for display */
            char *p = strstr(reply, "|");
            if (p) p = p + 1;
            if (p && p[0] >= '0' && p[0] <= '9')
            {
                p = strchr(p, '|');
                if (p) p++;
            }
            printf("\n%s\n\n", p ? p : reply);
        }
        else
        {
            printf("\nERROR: Server not responding.\n\n");
        }
    }

    close(s);
    return 0;
}   