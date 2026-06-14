# Compilador e Bibliotecas
CC = arm-linux-gnueabihf-gcc
LDLIBS = -lrt -lm

# Flags de compilação (-Iinclude avisa onde estão os arquivos .h)
CFLAGS = -Wall -Iinclude -std=c99 -D_POSIX_C_SOURCE=200809L

# Definição das pastas do projeto
SRC_DIR = src
BUILD_DIR = build

# Nome do executável final (agora salvo dentro da pasta build)
TARGET = $(BUILD_DIR)/exec

# Busca automaticamente todos os arquivos .c e .s dentro de src/
C_SOURCES = $(wildcard $(SRC_DIR)/*.c)
S_SOURCES = $(wildcard $(SRC_DIR)/*.s)

# Converte os nomes: ex: "src/vga.c" vira "build/vga.o"
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(C_SOURCES)) \
       $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(S_SOURCES))

# Regra padrãoCFLAGS = -Wall -Iinclude
all: $(BUILD_DIR) $(TARGET)

# Cria a pasta build automaticamente, caso ela não exista
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Regra para gerar o executável final
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

# Regra para compilar arquivos .c (joga os .o na pasta build)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para compilar arquivos .s (joga os .o na pasta build)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	$(CC) $(CFLAGS) -c $< -o $@

# Limpa tudo deletando a pasta build inteira
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)


