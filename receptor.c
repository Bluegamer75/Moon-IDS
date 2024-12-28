#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <mysql/mysql.h> // Envio de los datos a la base de datos

#define BUFFER_SIZE 1024

volatile sig_atomic_t keep_running = 1;

void intHandler(int dummy) {
    keep_running = 0;
}

// Función para conectarse a la base de datos MySQL
MYSQL *connect_to_database() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error al inicializar MySQL: %s\n", mysql_error(conn));
        return NULL;
    }

    // Conexión a la base de datos (modificar usuario, contraseña y nombre de la base de datos según corresponda)
    if (mysql_real_connect(conn, "127.0.0.1", "root", "", "moon", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error al conectar a la base de datos: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

// Función para insertar banners de puertos en la base de datos
void insert_port_banner(MYSQL *conn, const char *ip_address, int port, const char *banner) {
    char query[2048];
    snprintf(query, sizeof(query),
             "INSERT INTO port_banners (ip_address, port, banner) VALUES ('%s', %d, '%s');",
             ip_address, port, banner);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al insertar banner del puerto: %s\n", mysql_error(conn));
    } else {
        printf("Banner del puerto insertado correctamente.\n");
    }
}

int main(int argc, char *argv[]) {
    int listen_sockfd;
    struct sockaddr_in listen_addr;
    int port = 12345; // Puerto en el que el receptor escuchará

    if (argc > 1) {
        port = atoi(argv[1]); // Permite especificar el puerto como argumento
    }

    // Configurar el manejador de señales para SIGINT (Ctrl+C)
    signal(SIGINT, intHandler);

    // Conectar a la base de datos
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "No se pudo conectar a la base de datos. Cerrando receptor.\n");
        return EXIT_FAILURE;
    }

    // Crear el socket de escucha
    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sockfd < 0) {
        perror("Error al crear el socket de escucha");
        mysql_close(conn);
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
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    // Escuchar conexiones entrantes
    if (listen(listen_sockfd, 5) < 0) {
        perror("Error al escuchar en el socket");
        close(listen_sockfd);
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    printf("Receptor escuchando en el puerto %d...\n", port);

    while (keep_running) {
        int conn_sockfd;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        // Aceptar una conexión
        conn_sockfd = accept(listen_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (conn_sockfd < 0) {
            if (keep_running) {
                perror("Error al aceptar la conexión");
            }
            continue;
        }

        printf("Conexión aceptada desde %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Recibir datos
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received;

        while ((bytes_received = recv(conn_sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
            buffer[bytes_received] = '\0'; // Asegurarse de que el buffer sea una cadena válida
            printf("%s", buffer); // Imprimir los datos recibidos
            fflush(stdout);

            // Procesar solo el banner del servicio
            char ip_address[50];
            int port;
            char banner[1024];

            // Intentamos extraer el banner del buffer
            if (sscanf(buffer, "Puerto %d abierto en %49s\n%1023[^\n]", &port, ip_address, banner) == 3) {
                // Insertar solo el banner sin información extra
                insert_port_banner(conn, ip_address, port, banner);
            }
        }

        if (bytes_received < 0) {
            perror("Error al recibir datos");
        } else {
            printf("Conexión cerrada por el cliente\n");
        }

        // Cerrar socket de conexión
        close(conn_sockfd);
    }

    // Cerrar socket de escucha y conexión a la base de datos
    close(listen_sockfd);
    mysql_close(conn);
    printf("Receptor cerrado.\n");

    return 0;
}

// gcc -o receptor receptor.c -lmysqlclient
