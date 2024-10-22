
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
MEM_SRC = memory_manager.c
MEM_OBJ = memory_manager.o

LIST_SRC = linked_list.c
LIST_OBJ = linked_list.o

TESTER_SRC = test_linked_list.c
TESTER_OBJ = test_linked_list.o

# Default Target: Build both the memory manager library and the linked list application
all: mmanager list

# Compile the memory manager into a shared library
mmanager: $(MEM_SRC)
	$(CC) $(CFLAGS) -c $(MEM_SRC) -o $(MEM_OBJ)
	$(CC) $(LDFLAGS) -o $(LIBNAME) $(MEM_OBJ)

# Compile the linked list application and link it with the memory manager library
list: $(LIST_SRC) $(TESTER_SRC) mmanager
	$(CC) $(CFLAGS) -c $(LIST_SRC) -o $(LIST_OBJ)
	$(CC) $(CFLAGS) -c $(TESTER_SRC) -o $(TESTER_OBJ)
	$(CC) $(TESTER_OBJ) $(LIST_OBJ) -L. -lmemory_manager -o test_linked_list $(LIBS)

# Clean up generated files
clean:
	rm -f *.o $(LIBNAME) test_linked_list

# Phony Targets
.PHONY: all mmanager list clean
