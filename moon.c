#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define TIMEOUT 1  // Tiempo de espera en segundos
#define BUFSIZE 1024

// Función para escanear un puerto
int scan_port(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    
    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return 0;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return 0;  // Puerto cerrado
    }

    close(sockfd);
    return 1;  // Puerto abierto
}

// Función para obtener el banner de un servicio HTTP
void get_http_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Enviar solicitud HTTP
    send(sockfd, request, strlen(request), 0);

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner HTTP recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

// Función para obtener el banner de un servicio HTTPS
void get_https_banner(const char *ip, int port) {
    SSL_CTX *ctx;
    SSL *ssl;
    int sockfd;
    struct sockaddr_in server_addr;
    char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    char buffer[BUFSIZE];

    // Inicializar OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Error al crear el contexto SSL");
        return;
    }

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        SSL_CTX_free(ctx);
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        SSL_CTX_free(ctx);
        return;
    }

    // Establecer la conexión SSL
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    if (SSL_connect(ssl) != 1) {
        perror("Error en la conexión SSL");
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return;
    }

    // Enviar una solicitud HTTPS similar a una solicitud HTTP
    SSL_write(ssl, request, strlen(request));

    // Leer la respuesta del servidor
    int len = SSL_read(ssl, buffer, BUFSIZE - 1);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner HTTPS recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    // Limpiar
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
}

// Función para obtener el banner de un servicio FTP
void get_ftp_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner FTP recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

// Función para obtener el banner de un servicio SSH
void get_ssh_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Enviar una solicitud de conexión SSH (simple saludo para obtener el banner)
    send(sockfd, "SSH-1.99-OpenSSH_7.2p2 Ubuntu-4ubuntu2.8\r\n", 43, 0);

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner SSH recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

// Función para obtener el banner de CUPS
void get_cups_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char request[] = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Enviar solicitud HTTP
    send(sockfd, request, strlen(request), 0);

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner CUPS recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

// Función para obtener el banner de LDAP
void get_ldap_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor (usualmente el banner de LDAP se encuentra en el inicio)
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner LDAP recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

// Función para obtener el banner de TOR
void get_tor_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner TOR recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

// Función para obtener el banner de Minecraft
void get_minecraft_banner(const char *ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFSIZE];

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al conectar");
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner Minecraft recibido en puerto %d:\n%s\n", port, buffer);
    } else {
        printf("No se pudo obtener el banner del puerto %d\n", port);
    }

    close(sockfd);
}

void scan_ports(const char *ip, int start_port, int end_port) {
    for (int port = start_port; port <= end_port; ++port) {
        if (scan_port(ip, port)) {
            printf("Puerto %d abierto en %s\n", port, ip);

            // Llamamos a la función correspondiente según el puerto
            if (port == 80) {
                get_http_banner(ip, port);  // HTTP
            } else if (port == 443) {
                get_https_banner(ip, port);  // HTTPS
            } else if (port == 21) {
                get_ftp_banner(ip, port);  // FTP
            } else if (port == 22) {
                get_ssh_banner(ip, port);  // SSH
            } else if (port == 631) {
                get_cups_banner(ip, port);  // CUPS
            } else if (port == 389) {
                get_ldap_banner(ip, port);  // LDAP
            } else if (port == 9050 || port == 9001) {
                get_tor_banner(ip, port);  // TOR
            } else if (port == 25565) {
                get_minecraft_banner(ip, port);  // Minecraft
            }
        }
    }
}
void get_banner(const char *ip, int port) {
    char buffer[BUFSIZE];
    int sockfd;
    struct sockaddr_in server_addr;

    // Crear un socket TCP
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error al crear el socket");
        return;
    }

    // Configurar la dirección del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Intentar conectar al puerto
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        return;
    }

    // Leer la respuesta del servidor
    int len = recv(sockfd, buffer, BUFSIZE - 1, 0);
    if (len > 0) {
        buffer[len] = '\0';
        printf("Banner recibido en puerto %d:\n%s\n", port, buffer);
    }

    close(sockfd);
}

int main() {
    const char *target_ip = "127.0.0.1";  // Dirección IP a escanear
    int start_port = 1;    // Puerto inicial
    int end_port = 60000;   // Puerto final
    
    scan_ports(target_ip, start_port, end_port);

    return 0;
}



