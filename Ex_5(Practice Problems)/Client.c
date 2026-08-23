#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Socket creation error\n");
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return 1;
    }

    // Authentication input
    char user[50], pass[50];
    printf("Enter username: ");
    scanf("%s", user);
    printf("Enter password: ");
    scanf("%s", pass);

    write(sock, user, strlen(user) + 1);
    write(sock, pass, strlen(pass) + 1);

    read(sock, buffer, sizeof(buffer));
    if (strcmp(buffer, "SUCCESS") != 0) {
        printf("Authentication Failed! Incorrect username or password.\n");
        close(sock);
        return 1;
    }

    printf("\nLogin Successful! Connected to server.\n");

    while (1) {
        int choice;
        printf("\n--- TCP MULTI-SERVICE MENU ---\n");
        printf("1. Upload File\n");
        printf("2. Download File\n");
        printf("3. Get Server Date and Time\n");
        printf("4. Display Server System Information\n");
        printf("5. Terminate Session\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        write(sock, &choice, sizeof(choice));

        if (choice == 1) {
            char filename[256];
            printf("Enter filename to upload: ");
            scanf("%s", filename);
            write(sock, filename, sizeof(filename));

            FILE *fp = fopen(filename, "r");
            if (fp) {
                char file_buffer[1024];
                int n = fread(file_buffer, 1, sizeof(file_buffer), fp);
                write(sock, file_buffer, n);
                fclose(fp);
                read(sock, buffer, sizeof(buffer) - 1);
                printf("Server Response: %s\n", buffer);
            } else {
                printf("Local file not found.\n");
            }
        } 
        else if (choice == 2) {
            char filename[256];
            printf("Enter filename to download: ");
            scanf("%s", filename);
            write(sock, filename, sizeof(filename));

            int n = read(sock, buffer, sizeof(buffer) - 1);
            buffer[n] = '\0';
            printf("Server Response / File Content:\n%s\n", buffer);
        } 
        else if (choice == 3) {
            int n = read(sock, buffer, sizeof(buffer) - 1);
            buffer[n] = '\0';
            printf("Current Server Date and Time: %s\n", buffer);
        } 
        else if (choice == 4) {
            int n = read(sock, buffer, sizeof(buffer) - 1);
            buffer[n] = '\0';
            printf("Server System Info: %s\n", buffer);
        } 
        else if (choice == 5) {
            printf("Terminating session...\n");
            break;
        } 
        else {
            int n = read(sock, buffer, sizeof(buffer) - 1);
            buffer[n] = '\0';
            printf("Server Response: %s\n", buffer);
        }
    }

    close(sock);
    return 0;
}