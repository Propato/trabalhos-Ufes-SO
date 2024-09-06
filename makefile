# Nome do executável
TARGET = programa

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Lista de arquivos fonte
SRCS = main.c
# Lista de arquivos objeto
OBJS = $(SRCS:.c=.o)

# Regra padrão
all: $(TARGET)

# Regra para criar o executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Regra para compilar os arquivos .c em .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Regra para limpar os arquivos gerados
clean:
	rm -f $(TARGET) $(OBJS)

# Regra para executar o programa
run: $(TARGET)
	./$(TARGET)

# Impede que make trate 'clean' como um arquivo
.PHONY: all clean run
