# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LIBS = -lm -pthread

# Source and Object Files
SRC = memory_manager.c linked_list.c
OBJ = $(SRC:.c=.o)

# Default target
all: test_mmanager test_list

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the test_memory_manager executable
test_mmanager: test_memory_manager.o memory_manager.o
	$(CC) -o test_memory_manager test_memory_manager.o memory_manager.o $(LIBS)

# Build the test_linked_listCG executable
test_list: test_linked_list.o linked_list.o memory_manager.o
	$(CC) -o test_linked_listCG test_linked_list.o linked_list.o memory_manager.o $(LIBS)

# Clean target to clean up build files
clean:
	rm -f *.o test_memory_manager test_linked_listCG

# Phony Targets
.PHONY: all test_mmanager test_list clean
