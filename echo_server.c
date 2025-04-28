#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int port = 9001;
int verbose = 0;

void handleConnection(int* sock_fd_ptr) {
    int sock_fd = *sock_fd_ptr;
    free(sock_fd_ptr);

    if (verbose)
        printf("Handling connection on %d\n", sock_fd);
    char buffer[1024];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_read = read(sock_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read <= 0) {
            break;
        }

        if (verbose)
            printf("read %d bytes from %d: '%s'\n", bytes_read, sock_fd, buffer);

        write(sock_fd, buffer, bytes_read);
    }

    if (verbose)
        printf("done with connection %d\n", sock_fd);

    close(sock_fd);
    return;
}

int main(int argc, char* argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "p:v")) != -1) {
        switch (opt) {
            case 'p':
                port = atoi(optarg);
                break;
            case 'v':
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port] [-v]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in socket_address;
    memset(&socket_address, 0, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(port);

    int enable = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (bind(socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(socket_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if (verbose)
        printf("Server listening on port %d...\n", port);
    else
        printf("Server listening on port %d...\n", port);

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    while (1) {
        int* client_fd_buf = malloc(sizeof(int));
        *client_fd_buf = accept(socket_fd, (struct sockaddr*)&client_address, &client_address_len);
        if (*client_fd_buf < 0) {
            perror("accept");
            free(client_fd_buf);
            continue;
        }

        if (verbose)
            printf("accepted connection on %d\n", *client_fd_buf);

        pthread_t thread;
        pthread_create(&thread, NULL, (void* (*)(void*))handleConnection, (void*)client_fd_buf);
        pthread_detach(thread);
    }

    close(socket_fd);
    return 0;
}
