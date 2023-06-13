#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int student_number;
int client_socket;

void exit_program() {
    printf("\nClose socket and exit program\n");
    close(client_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Запускайте так: %s <Server IP Address> <Server Port><student_number>\n", argv[0]);
        exit_program();
    }
    struct sockaddr_in server_addr;
    if ((client_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Ошибка при создании сокета");
        exit_program();
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &server_addr.sin_addr);
    student_number = atoi(argv[3]);

    char buffer[1024];
    sprintf(buffer, "init");
    while (1) {
        struct sockaddr_in server_response;
        socklen_t addrlen = sizeof(server_response);
        int n, k, row_index;
        if (recvfrom(client_socket, &n, sizeof(n), 0, (struct sockaddr *) &server_response, &addrlen) < 0) {
            perror("recvfrom failed");
            exit_program();
        }
        if (n == -1) {
            break;
        }
        if (recvfrom(client_socket, &k, sizeof(k), 0, (struct sockaddr *) &server_response, &addrlen) < 0) {
            perror("recvfrom failed");
            exit_program();
        }
        if (recvfrom(client_socket, &row_index, sizeof(row_index), 0, (struct sockaddr *) &server_response,  &addrlen) < 0) {
            perror("recvfrom failed");
            exit_program();
        }

        int row[n * k];
        if (recvfrom(client_socket, row, n * k * sizeof(int), 0, (struct sockaddr *) &server_response, &addrlen) < 0) {
            perror("recvfrom failed");
            exit_program();
        }
        // Сортировка выбором
        for (int j = 0; j < n * k - 1; ++j) {
            int min = j;
            for (int l = j + 1; l < n * k; ++l) {
                if (row[l] < row[min]) {
                    min = l;
                }
            }
            int tmp = row[j];
            row[j] = row[min];
            row[min] = tmp;
            printf("Student %d have inserted book %d at the position %d of the bookshelf %d in a row %d. \n",
                   student_number,
                   row[j],
                   (j % k + 1), (j / n + 1), row_index + 1);
            usleep(rand() % 10);
        }
        if (send(client_socket, row, n * k * sizeof(int), 0) < 0) {
            perror("Send failed");
            exit(1);
        }
        printf("Student %d have finished sorting subcatalogue for row %d and passed it to the librarian.\n",
               student_number, row_index + 1);
        sprintf(buffer, "Student %d have finished sorting subcatalogue for row %d and passed it to the librarian.\n",
                student_number, row_index + 1);
        if (sendto(client_socket, row, n * k * sizeof(int), 0, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            perror("Send failed");
            exit(1);
        }
    }

    exit_program();
    return 0;
}
