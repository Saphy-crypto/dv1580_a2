// barrier.c
#include <pthread.h>
#include "memory_manager.h" // Ensure this header is available if needed

typedef struct {
    pthread_mutex_t mutex;  // Mutex to protect shared state
    pthread_cond_t cond;    // Condition variable to signal threads
    int count;              // Number of threads currently waiting
    int trip_count;         // Total number of threads that need to wait
} my_barrier_t;

/**
 * Initializes the custom barrier.
 * @param barrier Pointer to the barrier object
 * @param count Number of threads that need to wait before proceeding
 * @return 0 on success, non-zero on failure
 */
int my_barrier_init(my_barrier_t *barrier, unsigned int count) {
    if (count == 0) {
        return -1; // Invalid count
    }

    barrier->trip_count = count;
    barrier->count = 0;
    if (pthread_mutex_init(&barrier->mutex, NULL) != 0) {
        return -1;
    }
    if (pthread_cond_init(&barrier->cond, NULL) != 0) {
        pthread_mutex_destroy(&barrier->mutex);
        return -1;
    }
    return 0;
}

/**
 * Waits for all threads to reach the barrier before proceeding.
 * @param barrier Pointer to the barrier object
 * @return 0 on success
 */
int my_barrier_wait(my_barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mutex);
    barrier->count++;

    if (barrier->count >= barrier->trip_count) {
        barrier->count = 0; // Reset for reuse
        pthread_cond_broadcast(&barrier->cond); // Wake up all waiting threads
    } else {
        pthread_cond_wait(&barrier->cond, &barrier->mutex); // Wait until other threads reach the barrier
    }

    pthread_mutex_unlock(&barrier->mutex);
    return 0;
}

/**
 * Destroys the custom barrier and releases resources.
 * @param barrier Pointer to the barrier object
 * @return 0 on success
 */
int my_barrier_destroy(my_barrier_t *barrier) {
    pthread_mutex_destroy(&barrier->mutex);
    pthread_cond_destroy(&barrier->cond);
    return 0;
}
