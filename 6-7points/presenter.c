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

void exit_program() {
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

    char buffer[BUFFER_SIZE];
    int str_len;
    sprintf(buffer, "init");
    sendto(client_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *) &server_address, sizeof(server_address));

    printf("Connected to the server\n");

    while (1) {
        struct sockaddr_in server_response;
        socklen_t addrlen = sizeof(server_response);
        str_len = recvfrom(client_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *) &server_response, &addrlen);
        if (str_len > 0) {
            if (strcmp(buffer, "stop") == 0) {
                break;
            }
            printf("%s", buffer);
        }
    }
    exit_program();
}