#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PORT 9000
#define MAX 1024

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char questions[3][MAX] = {
    "1. What is the capital of India?\nA. Chennai\nB. Delhi\nC. Mumbai\nD. Kolkata\n",
    "2. Which language is used for system programming?\nA. HTML\nB. C\nC. CSS\nD. SQL\n",
    "3. What does CPU stand for?\nA. Central Processing Unit\nB. Computer Personal Unit\nC. Control Program Unit\nD. Central Program Utility\n"
};

char answers[3] = {'B', 'B', 'A'};

void log_activity(char message[])
{
    FILE *fp;
    time_t t;

    pthread_mutex_lock(&lock);

    fp = fopen("exam_log.txt", "a");

    t = time(NULL);

    fprintf(fp, "%s - %s", ctime(&t), message);

    fclose(fp);

    pthread_mutex_unlock(&lock);
}

void *client_handler(void *arg)
{
    int client = *(int *)arg;
    free(arg);

    char username[50];
    char password[50];
    char buffer[MAX];
    char log[MAX];
    int i;
    int score = 0;

    send(client, "Username: ", 10, 0);
    recv(client, username, sizeof(username), 0);
    username[strcspn(username, "\n")] = '\0';

    send(client, "Password: ", 10, 0);
    recv(client, password, sizeof(password), 0);
    password[strcspn(password, "\n")] = '\0';

    if (strcmp(username, "student") != 0 ||
        strcmp(password, "1234") != 0)
    {
        send(client, "Login failed!\n", 14, 0);

        sprintf(log, "%s - Login failed", username);
        log_activity(log);

        close(client);
        return NULL;
    }

    send(client, "Login successful!\n\n", 20, 0);

    sprintf(log, "%s - Login successful", username);
    log_activity(log);

    /* Send questions */

    for (i = 0; i < 3; i++)
    {
        send(client, questions[i], strlen(questions[i]), 0);
    }

    send(client, "\nEnter answers one by one:\n", 27, 0);

    /* Receive answers */

    for (i = 0; i < 3; i++)
    {
        recv(client, buffer, sizeof(buffer), 0);

        if (buffer[0] == answers[i])
            score++;
    }

    sprintf(buffer, "\nYour final score is %d/3\n", score);

    send(client, buffer, strlen(buffer), 0);

    sprintf(log, "%s - Exam completed - Score: %d/3",
            username, score);

    log_activity(log);

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

    printf("Online Examination Server running...\n");
    printf("Waiting for students...\n");

    while (1)
    {
        size = sizeof(client_addr);

        client = accept(server,
                        (struct sockaddr *)&client_addr,
                        &size);

        printf("Student connected.\n");

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
