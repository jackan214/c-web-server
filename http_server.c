#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

char* getRouting(char *arg) {
    size_t bufferSize = snprintf(NULL, 0, "./%s.html", arg) + 1;
    char* route = malloc(bufferSize);

    snprintf(route, bufferSize, "./%s.html", arg);

    return route;
}

void serveFile(int socket, const char filename) {
    char buffer[1024];
    int fd;
    int readBytes;

    fd = open(&filename, O_RDONLY);

    if (fd == -1) {
        perror("File open failed");
        return;
    }

    while ((readBytes = read(fd, buffer, sizeof(buffer))) > 0) {
        write(socket, buffer, readBytes);
    }

    close(fd);
}

void *handleClient(void *arg) {
    int socket = *(int*)arg;
    char buffer[1024];
    int readBytes;

    readBytes = read(socket, buffer, sizeof(buffer) - 1);

    buffer[readBytes] = '\0';
    printf("Received: %s\n", buffer);

    char *header = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
    write(socket, header, strlen(header));

    char *route = getRouting(buffer);

    serveFile(socket, *route);

    close(socket);

    free(arg);

    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("Waiting for connections...\n");

        int *client_fd = malloc(sizeof(int));

        if ((*client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen))<0) {
            perror("accept");
            continue;
        }
        
        pthread_t thread_id;

        if(pthread_create(&thread_id, NULL, handleClient, (void *)client_fd)) {
            perror("Could not create thread");
            continue;
        }

        pthread_detach(thread_id);        
    }

    return 0;
}
