#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    int s;
    char msg[256];

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

    printf("=== Arithmetic Client ===\n");
    printf("Operations: +  -  *  /\n");
    printf("Type 'exit' to quit.\n\n");

    while (1)
    {
        char op[10];
        double num1, num2;

        printf("Enter operation (+, -, *, /) or 'exit': ");
        fgets(op, sizeof(op), stdin);
        op[strcspn(op, "\n")] = '\0';

        if (strcmp(op, "exit") == 0)
        {
            sendto(s, "EXIT", 4, 0, (struct sockaddr*)&server, len);
            printf("Goodbye.\n");
            break;
        }

        printf("Enter first number: ");
        scanf("%lf", &num1);
        printf("Enter second number: ");
        scanf("%lf", &num2);
        while (getchar() != '\n');

        /* Build message: "op num1 num2" */
        sprintf(msg, "%s %.2f %.2f", op, num1, num2);

        sendto(s, msg, strlen(msg), 0,
               (struct sockaddr*)&server, len);

        /* Receive result */
        len = sizeof(server);
        recvfrom(s, msg, sizeof(msg), 0,
                 (struct sockaddr*)&server, &len);

        printf("Result: %s\n\n", msg);
    }

    close(s);
    return 0;
}   