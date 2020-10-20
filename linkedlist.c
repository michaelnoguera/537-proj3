/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file linkedlist.c
 */

#include "linkedlist.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

/**
 * Initializes a new empty LinkedList.
 * Remember to call ll_free when done!
 * 
 * @return pointer to new heap-allocated LinkedList
 */
LinkedList* ll_initialize() {
    LinkedList* list = (LinkedList*)malloc(sizeof(LinkedList));
    if (list == NULL) {
        perror("Error allocating memory for new list.\n");
        exit(EXIT_FAILURE);
    }

    list->size = 0;
    list->head = NULL;
    list->tail = NULL;

    return list;
}

/**
 * Adds a new node to the end of a LinkedList.
 * 
 * @param list The list to append to.
 * @param value Pointer to thing contained within the new node.
 */
void ll_push(LinkedList* list, void* value) {
    if (list == NULL) {
        perror("Can't add an element to NULL.");
        exit(EXIT_FAILURE);
    }

    // construct node to add
    struct ll_node_t* new = (struct ll_node_t*)malloc(sizeof(struct ll_node_t));
    if (new == NULL) {
        perror("Error allocating memory for new node.\n");
        exit(EXIT_FAILURE);
    }
    new->value = value;

    // attach node to tail of list
    if (list->size == 0) {
        list->head = new;
        list->tail = new;
    } else {
        list->tail->next = new;
        list->tail = new;
    }
    list->size++;

    new->next = list->head;  // complete circular linking
}

/**
 * Gets the value of the node at an index.
 * 
 * @param list The list to search.
 * @param index The index of the node to be retrieved. Lists are zero-indexed.
 * @return The value of the node, or NULL if no such node exists.
 */
void* ll_get(const LinkedList* list, int index) {
    if (list == NULL) {
        perror("Can't get an element from NULL.");
        exit(EXIT_FAILURE);
    }
    
    if (list->size <= index) return -1;

    struct ll_node_t* curr = list->head;
    for (int i = 0; i < index; i++) {
        curr = curr->next;
    }

    return curr->value;
}


/**
 * Frees all memory from the specified `LinkedList` and pointees of
 * the pointers it stores.
 * If  `ptr` is NULL, no operation is performed.
 * 
 * @param ptr Pointer to LinkedList to be freed.
 */
void ll_free(LinkedList* ptr) {
    if (ptr == NULL) return;
    
    // free all nodes
    struct ll_node_t* tmp = NULL;
    while (ptr->head != ptr->tail) {
        tmp = ptr->head;
        ptr->head = ptr->head->next;
        free(tmp->value);
        free(tmp);
    }
    free(ptr->tail->value);
    free(ptr->tail);

    // free list wrapper
    free(ptr);
}