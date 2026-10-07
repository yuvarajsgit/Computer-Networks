#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int s;
    int client_no = 1;
    char msg[256];

    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr*)&server, sizeof(server));

    printf("Arithmetic Server running on port %s\n", argv[1]);

    while (1)
    {
        len = sizeof(client);
        int n = recvfrom(s, msg, sizeof(msg), 0,
                         (struct sockaddr*)&client, &len);
        if (n <= 0) continue;

        if (fork() == 0)
        {
            close(s);

            /* Loop: handle multiple operations from this client */
            while (1)
            {
                if (strcmp(msg, "EXIT") == 0)
                    break;

                char op[10];
                double num1, num2;
                int parts = sscanf(msg, "%9s %lf %lf", op, &num1, &num2);

                if (parts != 3)
                {
                    strcpy(msg, "ERROR: Invalid input. Format: op num1 num2");
                }
                else
                {
                    double result;
                    if (strcmp(op, "+") == 0)
                        result = num1 + num2;
                    else if (strcmp(op, "-") == 0)
                        result = num1 - num2;
                    else if (strcmp(op, "*") == 0)
                        result = num1 * num2;
                    else if (strcmp(op, "/") == 0)
                    {
                        if (num2 == 0)
                        {
                            strcpy(msg, "ERROR: Division by zero");
                            sendto(s, msg, strlen(msg), 0,
                                   (struct sockaddr*)&client, len);
                            /* keep looping, wait for next op */
                            n = recvfrom(s, msg, sizeof(msg), 0,
                                         (struct sockaddr*)&client, &len);
                            if (n <= 0) break;
                            continue;
                        }
                        result = num1 / num2;
                    }
                    else
                    {
                        strcpy(msg, "ERROR: Unknown operation. Use +, -, *, /");
                        sendto(s, msg, strlen(msg), 0,
                               (struct sockaddr*)&client, len);
                        n = recvfrom(s, msg, sizeof(msg), 0,
                                     (struct sockaddr*)&client, &len);
                        if (n <= 0) break;
                        continue;
                    }

                    sprintf(msg, "%.2f", result);
                }

                sendto(s, msg, strlen(msg), 0,
                       (struct sockaddr*)&client, len);

                /* Wait for next operation */
                n = recvfrom(s, msg, sizeof(msg), 0,
                             (struct sockaddr*)&client, &len);
                if (n <= 0) break;
            }

            return 0;
        }

        client_no++;
    }

    return 0;
}   