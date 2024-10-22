# Compiler
CC = gcc

# Compiler Flags
CFLAGS = -Wall -Wextra -fPIC -pthread

# Library Flags
LDFLAGS = -shared

# Libraries
LIBS = -lm -pthread

# Targets
LIBNAME = libmemory_manager.so

# Source Files
MEM_SRC = memory_manager.c barrier.c
MEM_OBJ = memory_manager.o barrier.o

LIST_SRC = linked_list.c
LIST_OBJ = linked_list.o

TESTER_LINKED_SRC = test_linked_list.c
TESTER_LINKED_OBJ = test_linked_list.o

TESTER_MEMORY_SRC = test_memory_manager.c
TESTER_MEMORY_OBJ = test_memory_manager.o

# Default Target: Build the memory manager library, linked list application, and test executables
all: mmanager list test_linked_list test_memory_manager

# Compile the memory manager into a shared library
mmanager: $(MEM_SRC)
	$(CC) $(CFLAGS) -c memory_manager.c -o memory_manager.o
	$(CC) $(CFLAGS) -c barrier.c -o barrier.o
	$(CC) $(LDFLAGS) -o $(LIBNAME) memory_manager.o barrier.o

# Compile the linked list source
list: $(LIST_SRC) mmanager
	$(CC) $(CFLAGS) -c linked_list.c -o linked_list.o

# Compile test_linked_list executable
test_linked_list: $(TESTER_LINKED_SRC) linked_list.o mmanager
	$(CC) $(CFLAGS) -c test_linked_list.c -o test_linked_list.o
	$(CC) test_linked_list.o linked_list.o -L. -lmemory_manager -o test_linked_list $(LIBS)

# Compile test_memory_manager executable
test_memory_manager: $(TESTER_MEMORY_SRC) linked_list.o mmanager
	$(CC) $(CFLAGS) -c test_memory_manager.c -o test_memory_manager.o
	$(CC) test_memory_manager.o linked_list.o -L. -lmemory_manager -o test_memory_manager $(LIBS)

# Clean up generated files
clean:
	rm -f *.o $(LIBNAME) test_linked_list test_memory_manager

# Phony Targets
.PHONY: all mmanager list test_linked_list test_memory_manager clean
