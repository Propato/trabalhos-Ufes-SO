#		David Cristian Motta Propato
#			Elder Ribeiro Storck
#		 Jose Vitor Rodrigues Zorzal
#	   		    Eng. Comp. 
#	 Trabalho 1 de Sistemas Operacionais

# Nome do executável
TARGET = SO

# nome do arquivo principal (fora da pasta SRC)
MAIN = main

# Compilador e flags
CC = gcc
CFLAGS = -Wall -g -O3

# define lista de arquivos-fonte.
SOUCERS = $(wildcard ./SRC/*.c) ./SRC/$(MAIN).c

# define lista de arquivos-headers.
HEADERS = $(wildcard ./HEADERS/*.h)

# define lista dos arquivos-objeto usando nomes da lista de arquivos-fonte
OBJ_FOLDER = ./OBJ
OBJETOS = $(subst ./SRC,$(OBJ_FOLDER),$(SOUCERS:.c=.o))

############ ALVOS

# Regra padrão
all: $(OBJ_FOLDER) $(TARGET)

# gera pasta dos arquivos-objetos
$(OBJ_FOLDER):
	mkdir -p $@

# Regra para criar o executável
$(TARGET): $(OBJETOS)
	$(CC) $(CFLAGS) -o $@ $(OBJETOS)

# Regra para criar arquivos-objeto
$(OBJ_FOLDER)/%.o: SRC/%.c HEADERS/%.h
	$(CC) -c $(CFLAGS) $< -o $@

# Regra para criar arquivo-objeto do arquivo principal
$(OBJ_FOLDER)/$(MAIN).o: ./$(MAIN).c
	$(CC) -c $(CFLAGS) $< -o $@

# Regra para limpar os arquivos gerados
clean:
	rm -rf $(TARGET) $(OBJ_FOLDER)

# Regra para executar o programa
run: $(TARGET)
	./$(TARGET)

val: $(TARGET)
	valgrind -s --leak-check=full ./$(TARGET)

.PHONY: all clean run val
