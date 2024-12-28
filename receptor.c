#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <mysql/mysql.h>

#define BUFFER_SIZE 1024
#define MAX_DATA_SIZE 8192 // Tamaño máximo para acumular los datos completos

volatile sig_atomic_t keep_running = 1;

void intHandler(int dummy) {
    keep_running = 0;
}

MYSQL *connect_to_database() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Error al inicializar MySQL: %s\n", mysql_error(conn));
        return NULL;
    }

    if (mysql_real_connect(conn, "127.0.0.1", "root", "", "moon", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error al conectar a la base de datos: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    return conn;
}

// Función para insertar banner del puerto
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

// Función para insertar métricas del sistema
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

#define BUFFER_SIZE 1024


// Función para insertar las métricas en la base de datos
void insert_system_metrics(MYSQL *conn, float cpu_usage, float ram_usage, float disk_usage, const char *logged_users) {
    char query[2048];
    snprintf(query, sizeof(query),
             "INSERT INTO system_metrics (cpu_usage, ram_usage, disk_usage, logged_users) "
             "VALUES (%.2f, %.2f, %.2f, '%s');",
             cpu_usage, ram_usage, disk_usage, logged_users);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Error al insertar métricas del sistema: %s\n", mysql_error(conn));
    } else {
        printf("Métricas del sistema insertadas correctamente.\n");
    }
}

void process_system_metrics(MYSQL *conn, const char *data) {
    float cpu_usage = -1.0f;
    float ram_usage = -1.0f;
    float disk_usage = -1.0f;
    char logged_users[BUFFER_SIZE] = "";

    // Copiar los datos para procesarlos
    char *data_copy = strdup(data);
    if (data_copy == NULL) {
        fprintf(stderr, "Error al duplicar los datos.\n");
        return;
    }

    // Procesamos las métricas y los banners
    char *line = strtok(data_copy, "\n");
    while (line != NULL) {
        // Verificar si la línea contiene información de las métricas
        if (strstr(line, "Uso del procesador:") != NULL) {
            sscanf(line, "Uso del procesador: %f%%", &cpu_usage);
            printf("Uso del procesador: %.2f%%\n", cpu_usage);
        }
        if (strstr(line, "Uso de RAM:") != NULL) {
            sscanf(line, "Uso de RAM: %f%%", &ram_usage);
            printf("Uso de RAM: %.2f%%\n", ram_usage);
        }
        if (strstr(line, "Uso de disco:") != NULL) {
            sscanf(line, "Uso de disco: %f%%", &disk_usage);
            printf("Uso de disco: %.2f%%\n", disk_usage);
        }
        if (strstr(line, "Usuarios conectados:") != NULL) {
            // Aquí es donde los usuarios son procesados
            char *user_line = strtok(NULL, "\n");
            while (user_line != NULL) {
                strncat(logged_users, user_line, sizeof(logged_users) - strlen(logged_users) - 1);
                strncat(logged_users, "\n", sizeof(logged_users) - strlen(logged_users) - 1);
                user_line = strtok(NULL, "\n");
            }
            printf("Usuarios conectados:\n%s", logged_users);
        }
        line = strtok(NULL, "\n");
    }

    // Insertar métricas si son válidas
    if (cpu_usage != -1.0f || ram_usage != -1.0f || disk_usage != -1.0f) {
        insert_system_metrics(conn, cpu_usage, ram_usage, disk_usage, logged_users);
    }

    free(data_copy);
}

int main(int argc, char *argv[]) {
    int listen_sockfd;
    struct sockaddr_in listen_addr;
    int port = 12345;

    if (argc > 1) {
        port = atoi(argv[1]);
    }

    signal(SIGINT, intHandler);

    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "No se pudo conectar a la base de datos. Cerrando receptor.\n");
        return EXIT_FAILURE;
    }

    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sockfd < 0) {
        perror("Error al crear el socket de escucha");
        mysql_close(conn);
        return EXIT_FAILURE;
    }

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(port);

    if (bind(listen_sockfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("Error al enlazar el socket");
        close(listen_sockfd);
        mysql_close(conn);
        return EXIT_FAILURE;
    }

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

        conn_sockfd = accept(listen_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (conn_sockfd < 0) {
            if (keep_running) {
                perror("Error al aceptar la conexión");
            }
            continue;
        }

        printf("Conexión aceptada desde %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Buffer para acumular los datos
        char data_buffer[MAX_DATA_SIZE];
        size_t data_len = 0;

        ssize_t bytes_received;
        while ((bytes_received = recv(conn_sockfd, data_buffer + data_len, MAX_DATA_SIZE - data_len, 0)) > 0) {
            data_len += bytes_received;
            if (data_len >= MAX_DATA_SIZE || data_buffer[data_len - 1] == '\0') {
                break;
            }
        }

        if (bytes_received < 0) {
            perror("Error al recibir datos");
        } else {
            // Imprimir los datos completos recibidos
            printf("Datos completos recibidos:\n%s\n", data_buffer);

            // Procesar los banners de los puertos
            char ip_address[50];
            int port;
            char banner[1024];
            char *line = strtok(data_buffer, "\n");

            while (line != NULL) {
                printf("Procesando línea: %s\n", line);  // Imprimir la línea para depuración
                if (sscanf(line, "Puerto %d abierto en %49s", &port, ip_address) == 2) {
                    // Encontramos el puerto y la dirección IP
                    printf("Puerto %d abierto en IP %s\n", port, ip_address);
                } else if (sscanf(line, "Banner %1023[^\n]", banner) == 1) {
                    // Encontramos el banner
                    printf("Banner recibido: %s\n", banner);
                    insert_port_banner(conn, ip_address, port, banner);
                } else if (line[0] != '\0') {
                    // Si la línea no está vacía, puede ser un banner adicional
                    snprintf(banner, sizeof(banner), "%s", line);
                    printf("Banner adicional recibido: %s\n", banner);
                    insert_port_banner(conn, ip_address, port, banner);
                }

                // Si la línea contiene métricas del sistema, procesarlas
                if (strstr(line, "Uso del procesador:") || strstr(line, "Uso de RAM:") ||
                    strstr(line, "Uso de disco:") || strstr(line, "Usuarios conectados:")) {
                    process_system_metrics(conn, line);
                }

                line = strtok(NULL, "\n");
            }
        }

        printf("Conexión cerrada por el cliente\n");
        close(conn_sockfd);
    }

    close(listen_sockfd);
    mysql_close(conn);
    printf("Receptor cerrado.\n");

    return 0;
}



