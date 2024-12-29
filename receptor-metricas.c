#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <mysql/mysql.h>

#define BUFFER_SIZE 1024
#define MAX_DATA_SIZE 8192 // Tamaño máximo para acumular los datos completos

volatile sig_atomic_t keep_running = 1;

void intHandler(int dummy) {
    keep_running = 0;
}

// Conexión a la base de datos
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

// Inserción de banner del puerto
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

// Inserción de métricas del sistema
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

// Procesar datos de métricas
void process_system_metrics(MYSQL *conn, const char *data) {
    float cpu_usage = -1.0f;
    float ram_usage = -1.0f;
    float disk_usage = -1.0f;
    char logged_users[BUFFER_SIZE] = "";

    char *data_copy = strdup(data);
    if (data_copy == NULL) {
        fprintf(stderr, "Error al duplicar los datos.\n");
        return;
    }

    char *line = strtok(data_copy, "\n");
    while (line != NULL) {
        if (strstr(line, "Uso del procesador:") != NULL) {
            sscanf(line, "Uso del procesador: %f%%", &cpu_usage);
        }
        if (strstr(line, "Uso de RAM:") != NULL) {
            sscanf(line, "Uso de RAM: %f%%", &ram_usage);
        }
        if (strstr(line, "Uso de disco:") != NULL) {
            sscanf(line, "Uso de disco: %f%%", &disk_usage);
        }
        if (strstr(line, "Usuarios conectados:") != NULL) {
            char *user_line = strtok(NULL, "\n");
            while (user_line != NULL) {
                strncat(logged_users, user_line, sizeof(logged_users) - strlen(logged_users) - 1);
                strncat(logged_users, "\n", sizeof(logged_users) - strlen(logged_users) - 1);
                user_line = strtok(NULL, "\n");
            }
        }
        line = strtok(NULL, "\n");
    }

    if (cpu_usage != -1.0f || ram_usage != -1.0f || disk_usage != -1.0f) {
        insert_system_metrics(conn, cpu_usage, ram_usage, disk_usage, logged_users);
    }

    free(data_copy);
}

// Listener de métricas
void *metrics_listener(void *arg) {
    int metrics_port = *(int *)arg;
    MYSQL *conn = connect_to_database();
    if (conn == NULL) {
        fprintf(stderr, "No se pudo conectar a la base de datos. Cerrando hilo de métricas.\n");
        pthread_exit(NULL);
    }

    int listen_sockfd;
    struct sockaddr_in listen_addr;

    listen_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sockfd < 0) {
        perror("Error al crear el socket de escucha para métricas");
        mysql_close(conn);
        pthread_exit(NULL);
    }

    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(metrics_port);

    if (bind(listen_sockfd, (struct sockaddr *)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("Error al enlazar el socket de métricas");
        close(listen_sockfd);
        mysql_close(conn);
        pthread_exit(NULL);
    }

    if (listen(listen_sockfd, 5) < 0) {
        perror("Error al escuchar en el socket de métricas");
        close(listen_sockfd);
        mysql_close(conn);
        pthread_exit(NULL);
    }

    printf("Escuchando métricas en el puerto %d...\n", metrics_port);

    while (keep_running) {
        int conn_sockfd;
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);

        conn_sockfd = accept(listen_sockfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (conn_sockfd < 0) {
            if (keep_running) {
                perror("Error al aceptar la conexión de métricas");
            }
            continue;
        }

        char data_buffer[MAX_DATA_SIZE];
        ssize_t bytes_received = recv(conn_sockfd, data_buffer, sizeof(data_buffer) - 1, 0);
        if (bytes_received > 0) {
            data_buffer[bytes_received] = '\0';
            printf("Datos de métricas recibidos:\n%s\n", data_buffer);
            process_system_metrics(conn, data_buffer);
        }

        close(conn_sockfd);
    }

    close(listen_sockfd);
    mysql_close(conn);
    pthread_exit(NULL);
}

// Función principal
int main(int argc, char *argv[]) {
    int port = 12345;
    int metrics_port = 12346;

    if (argc > 1) {
        port = atoi(argv[1]);
    }
    if (argc > 2) {
        metrics_port = atoi(argv[2]);
    }

    signal(SIGINT, intHandler);

    pthread_t metrics_thread;
    pthread_create(&metrics_thread, NULL, metrics_listener, &metrics_port);

    printf("Hilo de métricas iniciado...\n");

    pthread_join(metrics_thread, NULL);
    printf("Servidor cerrado.\n");
    return 0;
}
