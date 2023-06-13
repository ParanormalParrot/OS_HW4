#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>

#define BUFFER_SIZE 1024

int client_socket;

void exit_program()
{
    printf("\nClose socket and exit program\n");
    close(client_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: <%s> <server ip> <sever port>\n", argv[0]);
        exit_program();
    }

    struct sockaddr_in server_address;
    signal(SIGINT, exit_program);

    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Error while creating socket\n");
        exit_program();
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0) {
        perror("Error while converting IP-address\n");
        exit_program();
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Error while connecting to the server\n");
        exit_program();
    }

    printf("Connected to the server\n");

    char buffer[BUFFER_SIZE];

    while (1) {
        str_len = recv(client_socket, &buffer, sizeof(buffer), 0);
        if (strcmp(buffer, "stop") == 0) {
            break;
        }
        printf(buffer);
    }
    exit_program();
}