# Variables
CC = gcc
CFLAGS = -Wall
LDFLAGS_SSL = -lssl -lcrypto -lssh
LDFLAGS_MYSQL = -lmysqlclient
SRC_DIR = src
TARGET_DIR = target/release
INSTALL_DIR = /usr/local/bin

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

# Regla para instalar los binarios en el sistema
install: all
	@echo "Instalando binarios en $(INSTALL_DIR)..."
	install -d $(INSTALL_DIR)   # Crear el directorio si no existe
	install $(TARGET_DIR)/moon $(INSTALL_DIR)
	install $(TARGET_DIR)/receptor-metricas $(INSTALL_DIR)
	install $(TARGET_DIR)/receptor-banner $(INSTALL_DIR)

# Limpiar los binarios generados
clean:
	rm -f $(TARGET_DIR)/moon $(TARGET_DIR)/receptor-metricas $(TARGET_DIR)/receptor-banner

# Crear el directorio de salida si no existe
$(TARGET_DIR):
	mkdir -p $(TARGET_DIR)

.PHONY: all clean install

