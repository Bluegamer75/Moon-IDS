#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    int listen_sockfd, conn_sockfd;
    struct sockaddr_in listen_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int port = 12345; // Puerto en el que el receptor escuchará

    if (argc > 1) {
        port = atoi(argv[1]); // Permite especificar el puerto como argumento
    }

    // Crear el socket de escucha
    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sockfd < 0) {
        perror("Error al crear el socket de escucha");
        return EXIT_FAILURE;
    }

    // Configurar la dirección del servidor
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(port);

    // Enlazar el socket
    if (bind(listen_sockfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("Error al enlazar el socket");
        close(listen_sockfd);
        return EXIT_FAILURE;
    }

    // Escuchar conexiones entrantes
    if (listen(listen_sockfd, 5) < 0) {
        perror("Error al escuchar en el socket");
        close(listen_sockfd);
        return EXIT_FAILURE;
    }

    printf("Receptor escuchando en el puerto %d...\n", port);

    // Aceptar una conexión
    conn_sockfd = accept(listen_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (conn_sockfd < 0) {
        perror("Error al aceptar la conexión");
        close(listen_sockfd);
        return EXIT_FAILURE;
    }

    printf("Conexión aceptada desde %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Recibir datos
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(conn_sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Asegurarse de que el buffer sea una cadena válida
        printf("%s", buffer); // Imprimir los datos recibidos
        fflush(stdout);
    }

    if (bytes_received < 0) {
        perror("Error al recibir datos");
    } else {
        printf("Conexión cerrada por el cliente\n");
    }

    // Cerrar sockets
    close(conn_sockfd);
    close(listen_sockfd);

    return 0;
}
