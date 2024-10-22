
#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>


// structure representing a block of memory within the pool.
//each block knows its starting address, size, and the next block in the list.
typedef struct MemBlock {
    void* start;            //starting address of the memory block
    size_t size;            //size of the memory block in bytes
    struct MemBlock* next;  //pointer to the next free or allocated block
} MemBlock;

//global variables managing the memory pool and tracking allocations.
//these are static to restrict their visibility to this file, ensuring encapsulation.
static char* memory_pool = NULL;   //pointer to the start of the memory pool
static size_t pool_size = 0;// total size of the memory pool
static MemBlock* free_list = NULL; // linked list of free memory blocks
static MemBlock* allocated_list = NULL;// linked list of allocated memory blocks
//static pthread_mutex_t mem_mutex;  // mutex to ensure thread-safe operations

static pthread_mutex_t mem_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for memory operations

/**
 * initializes the memory manager by setting up the memory pool and the initial free list.
 * this function should be called once at the beginning of the program.
 *
 * @param size the total size of the memory pool to allocate.
 */
void mem_init(size_t size) {
    // initialize the mutex to protect memory management operations.
   // pthread_mutex_init(&mem_mutex, NULL);

    // check if the memory manager is already initialized to prevent re-initialization.
    if (memory_pool != NULL) {
        fprintf(stderr, "Memory manager is already initialized.\n");
        return;
    }

    // allocate the memory pool using malloc.
    memory_pool = (char*)malloc(size);
    if (memory_pool == NULL) {
        fprintf(stderr, "Failed to allocate memory pool of size %zu.\n", size);
        exit(EXIT_FAILURE);
    }
    pool_size = size; //store the total pool size for boundary checks.

    //initialize the free list with a single block covering the entire pool.
    free_list = (MemBlock*)malloc(sizeof(MemBlock));
    if (free_list == NULL) {
        fprintf(stderr, "Failed to initialize free list.\n");
        free(memory_pool);
        memory_pool = NULL;
        exit(EXIT_FAILURE);
    }
    free_list->start = (void*)memory_pool; // start of the free block
    free_list->size = size;                // entire pool is initially free
    free_list->next = NULL;                // no other free blocks yet

    allocated_list = NULL; // no allocations have been made yet
}

/**
 *allocates a block of memory of the specified size from the memory pool.
 *it uses the first-fit strategy to find a suitable free block.
 *
 * @param size the size of memory to allocate in bytes.
 * @return a pointer to the allocated memory block, or NULL if allocation fails.
 */
void* mem_alloc(size_t size) {
    //lock the mutex to ensure exclusive access to the memory pool during allocation.
    pthread_mutex_lock(&mem_mutex);

    //ensure that the memory manager has been initialized.
    if (memory_pool == NULL) {
        fprintf(stderr, "Memory manager is not initialized.\n");
        pthread_mutex_unlock(&mem_mutex);
        return NULL;
    }

    MemBlock* current = free_list; //start searching from the beginning of the free list
    MemBlock* previous = NULL;     //to keep track of the previous block during traversal

    //traverse the free list to find the first block that fits the requested size.
    while (current != NULL) {
        if (current->size >= size) {
            void* allocated_ptr = current->start; // Address to return

            //create a new MemBlock to track this allocation.
            MemBlock* allocated_block = (MemBlock*)malloc(sizeof(MemBlock));
            if (allocated_block == NULL) {
                fprintf(stderr, "Failed to allocate memory for tracking.\n");
                pthread_mutex_unlock(&mem_mutex);
                return NULL;
            }
            allocated_block->start = allocated_ptr;
            allocated_block->size = size;
            allocated_block->next = allocated_list;//insert at the beginning of the allocated list
            allocated_list = allocated_block;

            //if the free block is larger than needed, split it
            if (current->size > size) {
                current->start = (char*)current->start + size; //move the start pointer forward
                current->size -= size; //decrease the size of the free block
            } else {
                //exact fit: Remove the block from the free list
                if (previous == NULL) {
                    free_list = current->next;
                } else {
                    previous->next = current->next;
                }
                free(current); //free the MemBlock structure as it's no longer needed
            }

            //unlock the mutex before returning the allocated memory
            pthread_mutex_unlock(&mem_mutex);
            return allocated_ptr;
        }
        previous = current;//move to the next block
        current = current->next;
    }

    //if no suitable block is found, print an error message
    fprintf(stderr, "No suitable block found for allocation of size %zu.\n", size);
    pthread_mutex_unlock(&mem_mutex);
    return NULL; //allocation failed
}

/**
 * frees a previously allocated memory block, returning it to the free list
 * it also attempts to merge adjacent free blocks to reduce fragmentation
 *
 * @param ptr pointer to the memory block to free
 */
void mem_free(void* ptr) {
    //check for NULL pointers to prevent undefined behavior
    if (ptr == NULL) {
        fprintf(stderr, "Attempted to free a NULL pointer.\n");
        return;
    }

    //lock the mutex to ensure exclusive access during deallocation
    pthread_mutex_lock(&mem_mutex);

    // verify that the pointer lies within the bounds of the memory pool
    if (ptr < (void*)memory_pool || ptr >= (void*)(memory_pool + pool_size)) {
        fprintf(stderr, "Attempted to free a pointer outside the memory pool.\n");
        pthread_mutex_unlock(&mem_mutex);
        return;
    }

    MemBlock* current = allocated_list; //start from the allocated list
    MemBlock* previous = NULL;

    //search for the allocated block corresponding to the pointer
    while (current != NULL) {
        if (current->start == ptr) {
            //remove the block from the allocated list
            if (previous == NULL) {
                allocated_list = current->next;
            } else {
                previous->next = current->next;
            }

            //create a new free block to return the memory.
            MemBlock* new_free = (MemBlock*)malloc(sizeof(MemBlock));
            if (new_free == NULL) {
                fprintf(stderr, "Failed to allocate memory for free block.\n");
                free(current); //free the allocated block structure even on failure
                pthread_mutex_unlock(&mem_mutex);
                return;
            }
            new_free->start = ptr;
            new_free->size = current->size;
            new_free->next = NULL; // wll be inserted into the free list

            free(current); // free the MemBlock structure as it's no longer needed

            //insert the new free block into the free list in address order.
            if (free_list == NULL || new_free->start < free_list->start) {
                //insert at the beginning if it's the first block or comes before the current first block
                new_free->next = free_list;
                free_list = new_free;
            } else {
                //find the correct position to insert to maintain order
                MemBlock* fcurrent = free_list;
                while (fcurrent->next != NULL && fcurrent->next->start < new_free->start) {
                    fcurrent = fcurrent->next;
                }
                new_free->next = fcurrent->next;
                fcurrent->next = new_free;
            }

            // attempt to merge adjacent free blocks to minimize fragmentation
            MemBlock* merge_current = free_list;
            while (merge_current != NULL && merge_current->next != NULL) {
                char* end_of_current = (char*)merge_current->start + merge_current->size;
                if (end_of_current == (char*)merge_current->next->start) {
                    //adjacent blocks found, merge them
                    merge_current->size += merge_current->next->size;
                    MemBlock* temp = merge_current->next;
                    merge_current->next = temp->next;
                    free(temp); // free the merged block's MemBlock structure
                } else {
                    merge_current = merge_current->next; // move to the next block
                }
            }

            //unlock the mutex after deallocation and merging
            pthread_mutex_unlock(&mem_mutex);
            return;
        }
        previous = current; //move to the next allocated block
        current = current->next;
    }

    // if the pointer was not found in the allocated list, it's an invalid free attempt
    fprintf(stderr, "Attempted to free a pointer that was not allocated.\n");
    pthread_mutex_unlock(&mem_mutex);
}

/**
 * resizes an allocated memory block to a new size
 * if the new size is larger, it attempts to allocate a new block and copy the data
 * if the new size is smaller, it adjusts the block size accordingly
 *
 * @param ptr pointer to the memory block to resize
 * @param size new size of the memory block in bytes
 * @return pointer to the resized memory block, or null if resizing fails
 */
void* mem_resize(void* ptr, size_t size) {
    // If the pointer is NULL, behave like mem_alloc
    if (ptr == NULL) {
        return mem_alloc(size);
    }
    // If the new size is zero, free the block
    if (size == 0) {
        mem_free(ptr);
        return NULL;
    }

    // Lock the mutex to ensure exclusive access during resizing
    pthread_mutex_lock(&mem_mutex);

    MemBlock* current = allocated_list;

    // Search for the allocated block corresponding to the pointer
    while (current != NULL) {
        if (current->start == ptr) {
            if (current->size >= size) {
                // Current block is already large enough; no need to resize
                pthread_mutex_unlock(&mem_mutex);
                return ptr;
            } else {
                // Current block is too small, need to allocate a new block
                // Allocate a new block while still holding the mutex
                void* new_ptr = mem_alloc(size);
                if (new_ptr == NULL) {
                    // Allocation failed, cannot resize
                    pthread_mutex_unlock(&mem_mutex);
                    return NULL;
                }

                // Copy the data from the old block to the new block
                memcpy(new_ptr, ptr, current->size);

                // Free the old block
                mem_free(ptr);

                pthread_mutex_unlock(&mem_mutex);
                return new_ptr; // Return the pointer to the new block
            }
        }
        current = current->next;
    }

    // If the pointer was not found in the allocated list, it's an invalid resize attempt
    fprintf(stderr, "Attempted to resize a pointer that was not allocated.\n");
    pthread_mutex_unlock(&mem_mutex);
    return NULL;
}


/**
 * deinitializes the memory manager by freeing the memory pool and all associated structures
 * this function should be called once at the end of the program
 */
void mem_deinit() {
    //lock the mutex to ensure exclusive access during deinitialization
    pthread_mutex_lock(&mem_mutex);

    if (memory_pool != NULL) {
        free(memory_pool); // Free the memory pool
        memory_pool = NULL; // Reset the pool pointer
        pool_size = 0; // Reset the pool size
    }

    // Free all blocks in the free list
    while (free_list != NULL) {
        MemBlock* temp = free_list;
        free_list = free_list->next;
        free(temp);
    }

    // Free all blocks in the allocated list
    while (allocated_list != NULL) {
        MemBlock* temp = allocated_list;
        allocated_list = allocated_list->next;
        free(temp);
    }

    // unlock the mutex after deallocation and merging
    pthread_mutex_unlock(&mem_mutex);

    // pthread_mutex_destroy(&mem_mutex);
}
