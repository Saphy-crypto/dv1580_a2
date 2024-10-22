#include "linked_list.h"
#include "memory_manager.h" 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>


//mutex to ensure that linked list operations are thread-safe
//this mutex protects all operations that modify or traverse the list
static pthread_mutex_t list_mutex;

/**
 * initializes the linked list and the memory manager
 * this function should be called once before any list operations are performed
 *
 * @param head double pointer to the head of the linked list
 * @param size the size of the memory pool to initialize the memory manager with
 */
void list_init(Node** head, size_t size) {
    // initialize the mutex to protect linked list operations
    pthread_mutex_init(&list_mutex, NULL);

    // initialize the custom memory manager with the specified pool size
    mem_init(size);

    //set the head of the list to NULL, indicating an empty list
    *head = NULL;
}

/**
 * inserts a new node with the specified data at the end of the linked list
 * this function is thread-safe and can be called concurrently by multiple threads
 *
 * @param head double pointer to the head of the linked list
 * @param data the data value to store in the new node
 */
void list_insert(Node** head, uint16_t data) {
    //lock the mutex to ensure exclusive access during insertion
    pthread_mutex_lock(&list_mutex);

    //allocate memory for the new node using the custom memory manager
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        //allocation failed,unlock the mutex and exit the function
        pthread_mutex_unlock(&list_mutex);
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        return;
    }
    new_node->data = data;//set the node's data.
    new_node->next = NULL; //new node will be the last node, so next is null

    if (*head == NULL) {
        // if the list is empty, the new node becomes the head
        *head = new_node;
    } else {
        //traverse to the end of the list to insert the new node
        Node* current = *head;
        while (current->next != NULL)
            current = current->next;
        current->next = new_node; //link the new node at the end
    }

    //unlock the mutex after insertion is complete
    pthread_mutex_unlock(&list_mutex);
}

/**
 * inserts a new node with the specified data after a given node
 * this function is thread-safe and can be called concurrently by multiple threads
 *
 * @param prev_node pointer to the node after which the new node will be inserted
 * @param data the data value to store in the new node
 */
void list_insert_after(Node* prev_node, uint16_t data) {
    if (prev_node == NULL) {
        //if the previous node is NULL, there's nothing to insert after
        fprintf(stderr, "Previous node is NULL. Cannot insert after.\n");
        return;
    }

    // lock the mutex to ensure exclusive access during insertion
    pthread_mutex_lock(&list_mutex);

    // allocate memory for the new node using the custom memory manager
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        // allocation failed; unlock the mutex and exit the function
        pthread_mutex_unlock(&list_mutex);
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        return;
    }
    new_node->data = data; //set the node's data.
    new_node->next = prev_node->next;//link the new node to the next node
    prev_node->next = new_node; //link the previous node to the new node

    // unlock the mutex after insertion is complete
    pthread_mutex_unlock(&list_mutex);
}

/**
 * inserts a new node with the specified data before a given node
 * this function is thread-safe and can be called concurrently by multiple threads
 *
 * @param head double pointer to the head of the linked list
 * @param next_node pointer to the node before which the new node will be inserted
 * @param data the data value to store in the new node
 */
void list_insert_before(Node** head, Node* next_node, uint16_t data) {
    if (next_node == NULL) {
        //if the next node is NULL, there's nothing to insert before
        fprintf(stderr, "Next node is NULL. Cannot insert before.\n");
        return;
    }

    // lock the mutex to ensure exclusive access during insertion.
    pthread_mutex_lock(&list_mutex);

    //allocate memory for the new node using the custom memory manager
    Node* new_node = (Node*)mem_alloc(sizeof(Node));
    if (new_node == NULL) {
        //allocation failed; unlock the mutex and exit the function
        pthread_mutex_unlock(&list_mutex);
        fprintf(stderr, "Failed to allocate memory for new node.\n");
        return;
    }
    new_node->data = data; //set the nodes data

    if (*head == next_node) {
        // if inserting before the head, update the head pointer
        new_node->next = *head;
        *head = new_node;
    } else {
        //traverse the list to find the node preceding the next_node
        Node* current = *head;
        while (current != NULL && current->next != next_node)
            current = current->next;

        if (current == NULL) {
            // next_node was not found in the list,clean up and exit
            fprintf(stderr, "Next node not found in the list. Cannot insert before.\n");
            mem_free(new_node);
            pthread_mutex_unlock(&list_mutex);
            return;
        }

        //link the new node into the list
        new_node->next = next_node;
        current->next = new_node;
    }

    // unlock the mutex after insertion is complete
    pthread_mutex_unlock(&list_mutex);
}

/**
 * deletes the first node in the linked list that contains the specified data
 * this function is thread-safe and can be called concurrently by multiple threads
 *
 * @param head double pointer to the head of the linked list
 * @param data the data value to search for in the list
 */
void list_delete(Node** head, uint16_t data) {
    //lock the mutex to ensure exclusive access during deletion
    pthread_mutex_lock(&list_mutex);

    if (*head == NULL) {
        //if the list is empty, there's nothing to delete
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    Node* temp = *head; //temporary pointer to traverse the list
    Node* prev = NULL; //pointer to keep track of the previous node

    //check if the head node contains the data to be deleted
    if (temp->data == data) {
        *head = temp->next; // update the head to the next node
        mem_free(temp); //free the memory of the old head
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    //traverse the list to find the node containing the data
    while (temp != NULL && temp->data != data) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        //the data was not found in the list; nothing to delete
        pthread_mutex_unlock(&list_mutex);
        return;
    }

    //unlink the node from the list and free its memory
    prev->next = temp->next;
    mem_free(temp);

    //unlock the mutex after deletion is complete
    pthread_mutex_unlock(&list_mutex);
}

/**
 * searches the linked list for the first node containing the specified data
 * this function is thread-safe and can be called concurrently by multiple threads
 *
 * @param head double pointer to the head of the linked list
 * @param data the data value to search for in the list
 * @return pointer to the found node, or null if not found
 */
Node* list_search(Node** head, uint16_t data) {
    //lock the mutex to ensure safe traversal during search
    pthread_mutex_lock(&list_mutex);

    Node* current = *head;//start from the head of the list
    while (current != NULL) {
        if (current->data == data) {
            // data found, unlock the mutex and return the node
            pthread_mutex_unlock(&list_mutex);
            return current;
        }
        current = current->next;//move to the next node
    }

    //data not found; unlock the mutex and return null
    pthread_mutex_unlock(&list_mutex);
    return NULL;
}

/**
 * displays all elements in the linked list in a readable format
 * this function is thread-safe and can be called concurrently by multiple threads
 *
 * @param head double pointer to the head of the linked list
 */
void list_display(Node** head) {
    // lock the mutex to ensure safe traversal during display
    pthread_mutex_lock(&list_mutex);

    Node* current = *head; //start from the head of the list
    printf("[");
    while (current != NULL) {
        printf("%u", current->data);//print the data of the current node
        if (current->next != NULL)
            printf(", "); //add a comma separator if not the last node
        current = current->next; //move to the next node
    }
    printf("]\n"); //end the display with a newline character

    //unlock the mutex after display is complete
    pthread_mutex_unlock(&list_mutex);
}

/**
 * displays elements in the linked list between two specified nodes, inclusive
 * this function is thread-safe and can be called concurrently by multiple threads
 *
 * @param head double pointer to the head of the linked list
 * @param start_node pointer to the node where the display should start
 * @param end_node pointer to the node where the display should end
 */
void list_display_range(Node** head, Node* start_node, Node* end_node) {
    //lock the mutex to ensure safe traversal during display
    pthread_mutex_lock(&list_mutex);

    Node* current = *head; //start from the head of the list.
    int in_range = (start_node == NULL) ? 1 : 0; // flag to track if within the range
    printf("[");
    while (current != NULL) {
        if (current == start_node)
            in_range = 1;//start displaying when start_node is reached
        if (in_range) {
            printf("%u", current->data); //print the data of the current node
            if (current == end_node)
                break; //stop displaying after end_node
            if (current->next != NULL)
                printf(", "); //add a comma separator if not the last node in range
        }
        current = current->next; //move to the next node
    }
    printf("]\n"); //end the display with a newline character

    //unlock the mutex after display is complete
    pthread_mutex_unlock(&list_mutex);
}

/**
 * counts the number of nodes present in the linked list
 * this function is thread-safe and can be called concurrently by multiple threads
 *
 * @param head double pointer to the head of the linked list
 * @return the total number of nodes in the list
 */
int list_count_nodes(Node** head) {
    //lock the mutex to ensure safe traversal during counting
    pthread_mutex_lock(&list_mutex);

    Node* current = *head; //start from the head of the list
    int count = 0;         //initialize the node counter
    while (current != NULL) {
        count++;            //increment the counter for each node
        current = current->next; //move to the next node
    }

    // unlock the mutex after counting is complete
    pthread_mutex_unlock(&list_mutex);
    return count; //return the total count
}

/**
 * cleans up the entire linked list by freeing all its nodes and deinitializing the memory manager
 * this function should be called once when the linked list is no longer needed
 *
 * @param head double pointer to the head of the linked list
 */
void list_cleanup(Node** head) {
    //lock the mutex to ensure exclusive access during cleanup
    pthread_mutex_lock(&list_mutex);

    Node* current = *head;//start from the head of the list
    while (current != NULL) {
        Node* next = current->next; //keep track of the next node
        mem_free(current); //free the current node's memory
        current = next;  //move to the next node
    }
    *head = NULL; // reset the head pointer to indicate an empty list

    //deinitialize the memory manager as it's no longer needed
    mem_deinit();

    //unlock the mutex after cleanup is complete
    pthread_mutex_unlock(&list_mutex);

    //destroy the mutex as it's no longer needed
    pthread_mutex_destroy(&list_mutex);
}