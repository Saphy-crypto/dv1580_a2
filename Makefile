# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -Wextra -g -fPIC $(EXTRA_CFLAGS)
LDFLAGS = -Wl,-rpath,.

LIB_NAME = libmemory_manager.so

# Libraries
LIBS = -lm -pthread

# Source and Object Files
SRC = memory_manager.c
OBJ = $(SRC:.c=.o)

# Default target
all: mmanager list test_mmanager test_list test_listCG

# Rule to create the dynamic library
$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $@ $(OBJ)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the memory manager
mmanager: $(LIB_NAME)

# Build the linked list object file
list: linked_list.o

# Build the test_memory_manager executable
test_mmanager: test_memory_manager.o $(LIB_NAME)
	$(CC) -o test_memory_manager test_memory_manager.o -L. -lmemory_manager $(LIBS) $(LDFLAGS)

# Build the test_linked_list executable
test_list: test_linked_list.o linked_list.o $(LIB_NAME)
	$(CC) -o test_linked_list test_linked_list.o linked_list.o -L. -lmemory_manager $(LIBS) $(LDFLAGS)

# Build the test_linked_listCG executable
test_listCG: test_linked_listCG.o linked_list.o $(LIB_NAME)
	$(CC) -o test_linked_listCG test_linked_listCG.o linked_list.o -L. -lmemory_manager $(LIBS) $(LDFLAGS)

# Clean target to clean up build files
clean:
	rm -f *.o $(LIB_NAME) test_memory_manager test_linked_list test_linked_listCG

# Phony Targets
.PHONY: all mmanager list test_mmanager test_list test_listCG clean
