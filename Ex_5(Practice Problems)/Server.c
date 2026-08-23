#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>

void log_activity(const char *activity, const char *client_ip, int client_port) {
    FILE *f = fopen("server_log.txt", "a");
    if (f != NULL) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strcspn(time_str, "\n")] = '\0';
        fprintf(f, "[%s] Client IP: %s, Port: %d - Action: %s\n", time_str, client_ip, client_port, activity);
        fclose(f);
    }
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    char buffer[1024];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        return 1;
    }

    printf("TCP Multi-Service Server is running on port 8080...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        int client_port = ntohs(client_addr.sin_port);

        printf("\nNew connection established.\n");

        // Authentication step
        char user[50], pass[50];
        read(client_fd, user, sizeof(user));
        read(client_fd, pass, sizeof(pass));

        if (strcmp(user, "admin") == 0 && strcmp(pass, "password123") == 0) {
            char *auth_success = "SUCCESS";
            write(client_fd, auth_success, strlen(auth_success));
            printf("[AUTH] Successful login from IP: %s, Port: %d\n", client_ip, client_port);
            log_activity("Successful Login", client_ip, client_port);
        } else {
            char *auth_fail = "FAILURE";
            write(client_fd, auth_fail, strlen(auth_fail));
            printf("[AUTH] Failed login attempt from IP: %s, Port: %d\n", client_ip, client_port);
            log_activity("Failed Login Attempt", client_ip, client_port);
            close(client_fd);
            continue;
        }

        // Service loop
        while (1) {
            int choice;
            int bytes_read = read(client_fd, &choice, sizeof(choice));
            if (bytes_read <= 0) break;

            if (choice == 1) { // Upload File
                char filename[256];
                read(client_fd, filename, sizeof(filename));
                FILE *fp = fopen(filename, "wb");
                if (fp) {
                    char file_buffer[1024];
                    int n = read(client_fd, file_buffer, sizeof(file_buffer));
                    fwrite(file_buffer, 1, n, fp);
                    fclose(fp);
                    write(client_fd, "File uploaded successfully", 27);
                    log_activity("Uploaded File", client_ip, client_port);
                } else {
                    write(client_fd, "Error uploading file", 20);
                }
            } 
            else if (choice == 2) { // Download File
                char filename[256];
                read(client_fd, filename, sizeof(filename));
                FILE *fp = fopen(filename, "rb");
                if (fp) {
                    char file_buffer[1024];
                    int n = fread(file_buffer, 1, sizeof(file_buffer), fp);
                    write(client_fd, file_buffer, n);
                    fclose(fp);
                    log_activity("Downloaded File", client_ip, client_port);
                } else {
                    write(client_fd, "File not found on server", 24);
                }
            } 
            else if (choice == 3) { // Date and Time
                time_t now = time(NULL);
                char *time_str = ctime(&now);
                write(client_fd, time_str, strlen(time_str));
                log_activity("Retrieved Date and Time", client_ip, client_port);
            } 
            else if (choice == 4) { // Server System Info
                char sys_info[256] = "OS: Linux (WSL), Architecture: x86_64, Status: Online";
                write(client_fd, sys_info, strlen(sys_info));
                log_activity("Retrieved System Info", client_ip, client_port);
            } 
            else if (choice == 5) { // Terminate Session
                log_activity("Terminated Session", client_ip, client_port);
                break;
            } 
            else {
                write(client_fd, "Invalid request", 16);
                log_activity("Invalid Request Handled", client_ip, client_port);
            }
        }

        close(client_fd);
        printf("Client session closed.\n");
    }

    close(server_fd);
    return 0;
}