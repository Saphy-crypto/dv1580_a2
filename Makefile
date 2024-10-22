# Compiler and Linking Variables
CC = gcc
CFLAGS = -Wall -Wextra -fPIC
LIB_NAME = libmemory_manager.so

# Libraries
LIBS = -lm -pthread

# Source and Object Files
SRC = memory_manager.c
OBJ = $(SRC:.c=.o)

# Additional Source Files
LINKED_LIST_SRC = linked_list.c
LINKED_LIST_OBJ = linked_list.o

TEST_MEMORY_MANAGER_SRC = test_memory_manager.c
TEST_MEMORY_MANAGER_OBJ = test_memory_manager.o

TEST_LINKED_LIST_SRC = test_linked_list.c
TEST_LINKED_LIST_OBJ = test_linked_list.o

# Default target
all: mmanager list test_mmanager test_list

# Rule to create the dynamic library
$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $@ $(OBJ)

# Rule to compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the memory manager
mmanager: $(LIB_NAME)

# Build the linked list
list: $(LINKED_LIST_OBJ)

# Compile test_memory_manager.o
$(TEST_MEMORY_MANAGER_OBJ): $(TEST_MEMORY_MANAGER_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Test target to run the memory manager test program
test_mmanager: $(TEST_MEMORY_MANAGER_OBJ) $(LIB_NAME)
	$(CC) -o test_memory_manager $(TEST_MEMORY_MANAGER_OBJ) -L. -lmemory_manager $(LIBS)

# Compile test_linked_list.o
$(TEST_LINKED_LIST_OBJ): $(TEST_LINKED_LIST_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Test target to run the linked list test program
test_list: $(TEST_LINKED_LIST_OBJ) $(LINKED_LIST_OBJ) $(LIB_NAME)
	$(CC) -o test_linked_list test_linked_list.o linked_list.o -L. -lmemory_manager $(LIBS)

# Run tests
run_tests: run_test_mmanager run_test_list

# Run test cases for the memory manager
run_test_mmanager:
	./test_memory_manager

# Run test cases for the linked list
run_test_list:
	./test_linked_list

# Clean target to clean up build files
clean:
	rm -f *.o $(LIB_NAME) test_memory_manager test_linked_list

# Phony Targets
.PHONY: all mmanager list test_mmanager test_list run_tests run_test_mmanager run_test_list clean
