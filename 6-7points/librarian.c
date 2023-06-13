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
    struct sockaddr_in sockets[number_of_students];
    struct sockaddr_in presenter_addr;
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

    printf("Waiting for presenter...\n");
    char buffer[BUFFER_SIZE];
    ssize_t recv_presenter = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&address, (socklen_t *)&addrlen);
    if (recv_presenter < 0) {
        printf("Error in recvfrom() function for client presenter\n");
        exit_program();
    }
    printf("Presenter connected\n");
    // Сохранение адреса и порта presenter
    memcpy(&presenter_addr, &address, sizeof(address));

    printf("Librarian is waiting for students...\n");

    for (int i = 0; i < number_of_students; ++i) {
        // Принятие входящего подключения
        char init_buffer[BUFFER_SIZE];
        memset(init_buffer, 0, sizeof(init_buffer));
        ssize_t recv_res = recvfrom(server_socket, init_buffer, sizeof(init_buffer), 0, (struct sockaddr *) &address,
                &addrlen);
        if (recv_res < 0) {
            printf("Error in recvfrom() function for client %d\n", i);
            exit_program();
        }
        printf("Client %d connected\n", i + 1);
        // Сохранение адреса и порта клиента
        memcpy(&sockets[i], &address, sizeof(address));

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
        sendto(server_socket, &n, sizeof(n), 0, (struct sockaddr *) &sockets[i % number_of_students],
               sizeof(struct sockaddr_in));
        sendto(server_socket, &k, sizeof(k), 0, (struct sockaddr *) &sockets[i % number_of_students],
               sizeof(struct sockaddr_in));
        sendto(server_socket, &i, sizeof(i), 0, (struct sockaddr *) &sockets[i % number_of_students],
               sizeof(struct sockaddr_in));
        sendto(server_socket, row, n * k * sizeof(int), 0, (struct sockaddr *) &sockets[i % number_of_students],
               sizeof(struct sockaddr_in));
        recvfrom(server_socket, row, n * k * sizeof(int), 0, (struct sockaddr *) &sockets[i % number_of_students],
                 sizeof(struct sockaddr_in));
        for (int j = 0; j <row_size; ++j) {
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
        sendto(server_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *)&presenter_addr, sizeof(struct sockaddr_in));
    }

    int exit_code = -1;
    for (int i = 0; i < number_of_students; ++i) {
        // Отправка кода клиенту длfя завершения работы программы
        sendto(server_socket, &exit_code, sizeof(int), 0, (struct sockaddr *) &sockets[i], sizeof(struct sockaddr_in));
    }
    sprintf(buffer, "stop");
    sendto(server_socket, &buffer, sizeof(buffer), 0, (struct sockaddr *)&presenter_addr, sizeof(struct sockaddr_in));
    exit_program();
}