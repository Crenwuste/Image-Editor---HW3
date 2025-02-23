# Compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# Define source and header files
SRC=tema3.c
HEADERS=utils.h
OBJ=$(SRC:.c=.o)

# Define target
TARGET=image_editor

# Default build target
build: $(TARGET)

# Build target from object files
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

# Compile individual source files into object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Packaging rule
pack:
	zip -FSr 313CA_TraistaruDragosAndrei_Tema3.zip README Makefile $(SRC) $(HEADERS)

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJ)

# Phony targets
.PHONY: build pack clean