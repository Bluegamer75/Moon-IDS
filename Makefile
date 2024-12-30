# Variables
CC = gcc
CFLAGS = -Wall
LDFLAGS_SSL = -lssl -lcrypto -lssh
LDFLAGS_MYSQL = -lmysqlclient
SRC_DIR = src
TARGET_DIR = target/release

# Nombres de los ejecutables
BINS = $(TARGET_DIR)/moon $(TARGET_DIR)/receptor-metricas $(TARGET_DIR)/receptor-banner

# Reglas de compilación
all: $(BINS)

$(TARGET_DIR)/moon: $(SRC_DIR)/moon.c | $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_SSL)

$(TARGET_DIR)/receptor-metricas: $(SRC_DIR)/receptor-metricas.c | $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_MYSQL)

$(TARGET_DIR)/receptor-banner: $(SRC_DIR)/receptor-banner.c | $(TARGET_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_MYSQL)

# Limpiar los binarios generados
clean:
	rm -f $(TARGET_DIR)/moon $(TARGET_DIR)/receptor-metricas $(TARGET_DIR)/receptor-banner

# Crear el directorio de salida si no existe
$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

.PHONY: all clean

