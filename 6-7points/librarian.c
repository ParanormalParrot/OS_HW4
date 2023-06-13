#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <time.h>


#define BUFFER_SIZE 1024

#define ARRAY_SIZE 1000

int server_socket;
int n, m, k, port;
int number_of_students;
int books[ARRAY_SIZE];

void exit_program() {
    printf("\nClose socket and exit program\n");
    close(server_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    if (argc != 6) {
        printf("Использование: %s  <порт> <m> <n> <k> <количество студентов>\n", argv[0]);
        exit_program();
    }
    port = atoi(argv[1]);
    m = atoi(argv[2]);
    n = atoi(argv[3]);
    k = atoi(argv[4]);
    number_of_students = atoi(argv[5]);
    if (m * n * k > 1000) {
        printf("Input values are too big\n");
        return 1;
    }
    signal(SIGINT, exit_program);

    ssize_t read_message;
    sockaddr_in sockets[number_of_students];
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Error while creating socket\n");
        exit_program();
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Error while binding socket\n");
        exit_program();
    }

    if (listen(server_socket, number_of_students) < 0) {
        perror("Error in listen function\n");
        exit_program();
    }

    printf("Waiting for presenter...\n");
    int presenter_socket;
    char buffer[BUFFER_SIZE];
    if ((presenter_socket = recvfrom(server_socket, (struct sockaddr *) &address, (socklen_t * ) & addrlen)) < 0) {
        printf("Error in accept() function for presenter\n");
        exit_program();
    }
    printf("Presenter connected\n");

    printf("Librarian is waiting for students...\n");

    for (int i = 0; i < number_of_students; ++i) {
        // Принятие входящего подключения
        if ((sockets[i] = accept(server_socket, (struct sockaddr *) &address, (socklen_t * ) & addrlen)) < 0) {
            printf("Error in accept() function for client %d\n", i);
            exit_program();
        }
        printf("Client %d connected\n", i);
    }
    for (int i = 0; i < m * n * k; ++i) {
        books[i] = rand() % 1000;
    }
    int row_size = n * k;
    for (int i = 0; i < m; ++i) {
        int row[n * k];
        for (int j = 0; j < row_size; ++j) {
            row[j] = books[j + i * n * k];
        }
        send(sockets[i % number_of_students], &n, sizeof(n), 0);
        send(sockets[i % number_of_students], &k, sizeof(k), 0);
        send(sockets[i % number_of_students], &i, sizeof(i), 0);
        send(sockets[i % number_of_students], row, n * k * sizeof(int), 0);
        recv(sockets[i % number_of_students], row, n * k * sizeof(int), 0);
        recv(sockets[i % number_of_students], buffer, sizeof(buffer), 0);
        send(presenter_socket, buffer, sizeof(buffer), 0);
        for (int j = 0; j < row_size; ++j) {
            books[j + i * n * k] = row[j];
        }
    }

    for (int i = 0; i < m * n * k - 1; ++i) {
        int min = i;
        for (int j = i + 1; j < m * n * k; ++j) {
            if (books[j] < books[min]) {
                min = j;
            }

        }
        int tmp = books[i];
        books[i] = books[min];
        books[min] = tmp;
    }

    printf("The librarian have completed the catalogue.\n");
    sprintf(buffer, "The librarian have completed the catalogue.\n");
    for (int i = 0; i < m * n * k; ++i) {
        printf("Book %d at the position %d of the bookshelf %d in the row %d.\n", books[i], (i % k) + 1,
               (i / k % n) + 1, (i / k / n) + 1);
        sprintf(buffer, "Book %d at the position %d of the bookshelf %d in the row %d.\n", books[i], (i % k) + 1,
                (i / k % n) + 1, (i / k / n) + 1);
        send(presenter_socket, &buffer, sizeof(buffer), 0);
    }

    int exit_code = -1;
    for (int i = 0; i < number_of_students; ++i) {
        send(sockets[i], &exit_code, sizeof(int), 0);
    }

    sprintf(buffer, "stop");
    send(presenter_socket, &buffer, sizeof(buffer), 0);
    exit_program();
}