#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define DB_FILE   "library.db"
#define LOG_FILE  "library.log"
#define MAX_TXN   1024

/* Track processed transaction IDs to avoid duplicates */
long processed_txn[MAX_TXN];
int txn_count = 0;

void init_db()
{
    FILE *f = fopen(DB_FILE, "r");
    if (!f)
    {
        f = fopen(DB_FILE, "w");
        fprintf(f, "B001|The C Programming Language|Kernighan & Ritchie|AVAILABLE\n");
        fprintf(f, "B002|Operating System Concepts|Silberschatz|AVAILABLE\n");
        fprintf(f, "B003|Database System Concepts|Silberschatz|AVAILABLE\n");
        fprintf(f, "B004|Computer Networks|Tanenbaum|AVAILABLE\n");
        fprintf(f, "B005|Artificial Intelligence|Russell & Norvig|AVAILABLE\n");
        fclose(f);
    }
    else
        fclose(f);
}

void log_activity(const char *action)
{
    FILE *f = fopen(LOG_FILE, "a");
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    fprintf(f, "[%s] %s\n", asctime(tm), action);
    fclose(f);
}

int is_duplicate(long txn_id)
{
    for (int i = 0; i < txn_count; i++)
        if (processed_txn[i] == txn_id)
            return 1;
    return 0;
}

void mark_processed(long txn_id)
{
    if (txn_count < MAX_TXN)
        processed_txn[txn_count++] = txn_id;
}

void handle_request(char *msg, char *reply, int *reply_len)
{
    long txn_id;
    char cmd[20], arg1[50], arg2[50];
    int parts = sscanf(msg, "%ld|%19s|%49s|%49s", &txn_id, cmd, arg1, arg2);

    /* Duplicate check */
    if (is_duplicate(txn_id))
    {
        sprintf(reply, "ACK|%ld|DUPLICATE: Already processed", txn_id);
        *reply_len = strlen(reply);
        return;
    }
    mark_processed(txn_id);

    if (parts < 3)
    {
        sprintf(reply, "ACK|%ld|ERROR: Invalid request format", txn_id);
        *reply_len = strlen(reply);
        return;
    }

    /* ---- LIST ---- */
    if (strcmp(cmd, "LIST") == 0)
    {
        FILE *f = fopen(DB_FILE, "r");
        char line[200];
        int count = 0;
        while (fgets(line, sizeof(line), f))
        {
            if (strstr(line, "AVAILABLE"))
                count++;
        }
        fclose(f);
        sprintf(reply, "ACK|%ld|AVAILABLE (%d books):\n", txn_id, count);
        *reply_len = strlen(reply);

        f = fopen(DB_FILE, "r");
        while (fgets(line, sizeof(line), f))
        {
            if (strstr(line, "AVAILABLE"))
            {
                line[strcspn(line, "\n")] = '\0';
                strcat(reply, "  ");
                strcat(reply, line);
                strcat(reply, "\n");
                *reply_len = strlen(reply);
            }
        }
        fclose(f);
        log_activity("LIST available books");
        return;
    }

    /* ---- SEARCH ---- */
    if (strcmp(cmd, "SEARCH") == 0)
    {
        FILE *f = fopen(DB_FILE, "r");
        char line[200];
        int found = 0;
        while (fgets(line, sizeof(line), f))
        {
            if (strstr(line, arg1))
            {
                found = 1;
                line[strcspn(line, "\n")] = '\0';
                sprintf(reply, "ACK|%ld|FOUND: %s", txn_id, line);
                *reply_len = strlen(reply);
                break;
            }
        }
        fclose(f);
        if (!found)
            sprintf(reply, "ACK|%ld|NOT FOUND: No book matching '%s'", txn_id, arg1);
        *reply_len = strlen(reply);
        log_activity("SEARCH for book");
        return;
    }

    /* ---- ISSUE ---- */
    if (strcmp(cmd, "ISSUE") == 0)
    {
        FILE *f = fopen(DB_FILE, "r");
        char line[200];
        char newdb[4096] = "";
        int found = 0, issued = 0;

        while (fgets(line, sizeof(line), f))
        {
            if (strstr(line, arg1))
            {
                found = 1;
                if (strstr(line, "AVAILABLE"))
                {
                    /* Mark as ISSUED */
                    char *p = strstr(line, "AVAILABLE");
                    memcpy(p, "ISSUED", 6);
                    issued = 1;
                }
                else
                {
                    sprintf(reply, "ACK|%ld|ERROR: Book not available", txn_id);
                    *reply_len = strlen(reply);
                    fclose(f);
                    log_activity("ISSUE failed - not available");
                    return;
                }
            }
            strcat(newdb, line);
        }
        fclose(f);

        if (!found)
            sprintf(reply, "ACK|%ld|ERROR: Book '%s' not found", txn_id, arg1);
        else if (issued)
            sprintf(reply, "ACK|%ld|SUCCESS: Book '%s' issued to member '%s'", txn_id, arg1, arg2);
        *reply_len = strlen(reply);

        if (issued)
        {
            FILE *wf = fopen(DB_FILE, "w");
            fputs(newdb, wf);
            fclose(wf);
        }
        log_activity("ISSUE book");
        return;
    }

    /* ---- RETURN ---- */
    if (strcmp(cmd, "RETURN") == 0)
    {
        FILE *f = fopen(DB_FILE, "r");
        char line[200];
        char newdb[4096] = "";
        int found = 0, returned = 0;

        while (fgets(line, sizeof(line), f))
        {
            if (strstr(line, arg1))
            {
                found = 1;
                if (strstr(line, "ISSUED"))
                {
                    char *p = strstr(line, "ISSUED");
                    memcpy(p, "AVAILABLE", 9);
                    returned = 1;
                }
            }
            strcat(newdb, line);
        }
        fclose(f);

        if (!found)
            sprintf(reply, "ACK|%ld|ERROR: Book '%s' not found", txn_id, arg1);
        else if (returned)
            sprintf(reply, "ACK|%ld|SUCCESS: Book '%s' returned by member '%s'", txn_id, arg1, arg2);
        else
            sprintf(reply, "ACK|%ld|ERROR: Book is not currently issued", txn_id);
        *reply_len = strlen(reply);

        if (returned)
        {
            FILE *wf = fopen(DB_FILE, "w");
            fputs(newdb, wf);
            fclose(wf);
        }
        log_activity("RETURN book");
        return;
    }

    sprintf(reply, "ACK|%ld|ERROR: Unknown command '%s'", txn_id, cmd);
    *reply_len = strlen(reply);
}

int main(int argc, char *argv[])
{
    int s;
    int client_no = 1;
    char msg[1024];

    if (argc < 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    init_db();

    s = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr*)&server, sizeof(server));

    printf("Library Server running on port %s\n", argv[1]);

    while (1)
    {
        len = sizeof(client);
        int n = recvfrom(s, msg, sizeof(msg), 0,
                         (struct sockaddr*)&client, &len);
        if (n <= 0) continue;

        if (fork() == 0)
        {
            close(s);

            /* Loop: handle multiple requests from this client */
            while (1)
            {
                if (strcmp(msg, "QUIT") == 0)
                    break;

                char reply[2048];
                int reply_len = 0;
                handle_request(msg, reply, &reply_len);

                sendto(s, reply, reply_len, 0,
                       (struct sockaddr*)&client, len);

                /* Wait for next request */
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