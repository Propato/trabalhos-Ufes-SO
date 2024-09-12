#		David Cristian Motta Propato
#			Elder Ribeiro Storck
#		 Jose Vitor Rodrigues Zorzal

#	   		 UFES Eng. Comp. 
#	 		Operating Systems

# When the shell is executed from the makefile, there are some errors with keyboard commands
# (as it is directed to the make process, not the shell process).

# Executable name
EXEC = FSH

# Main file name (outside the SRC folder)
MAIN = main

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -g -O3

# Defines the list of source files
SOUCER_FOLDER = ./SRC
SOUCERS = $(wildcard $(SOUCER_FOLDER)/*.c) $(SOUCER_FOLDER)/$(MAIN).c

# Defines the list of header files
HEADER_FOLDER = ./HEADERS
HEADERS = $(wildcard $(HEADER_FOLDER)/*.h)

# Define the list of object files using the list of source files
OBJ_FOLDER = ./OBJ
OBJECTS = $(subst $(SOUCER_FOLDER),$(OBJ_FOLDER),$(SOUCERS:.c=.o))

############ TARGETS

# Building
all: $(OBJ_FOLDER) $(EXEC)

# Makes the objects folder
$(OBJ_FOLDER):
	mkdir -p $@

# Makes the exec file
$(EXEC): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

# Makes the object files
$(OBJ_FOLDER)/%.o: $(SOUCER_FOLDER)/%.c $(HEADER_FOLDER)/%.h
	$(CC) -c $(CFLAGS) $< -o $@

# Makes the object file for the main file
$(OBJ_FOLDER)/$(MAIN).o: ./$(MAIN).c
	$(CC) -c $(CFLAGS) $< -o $@

# Cleaning
clean:
	rm -rf $(EXEC) $(OBJ_FOLDER)

# Running
run: $(EXEC)
	./$(EXEC)

# Running with Valgrind
val: $(EXEC)
	valgrind -s --leak-check=full --show-leak-kinds=all ./$(EXEC)

# Explicit declaration of command rules
.PHONY: all clean run val
