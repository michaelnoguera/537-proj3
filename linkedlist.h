/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file linkedlist.h
 * @brief Implementation of heap-allocated circular singly-linked list with head 
 * and tail pointers.
 * 
 * Nodes store `int` values. Order is not guaranteed when multiple nodes have the
 * same value, especially after remove operations.
 */

#include <stdlib.h>

#ifndef _LINKEDLIST_
#define _LINKEDLIST_

typedef struct ll_node_t {
    void* value; /**< the value stored in the node */ 
    struct ll_node_t* next; /**< pointer to the next node in the list */
} LinkedListNode;

typedef struct linkedlist_t {
    int size; /**< number of items currently in the list*/
    struct ll_node_t* head; /**< pointer to the head of the list*/
    struct ll_node_t* tail; /**< pointer to the tail of the list*/
} LinkedList;

LinkedList* ll_initialize();

void ll_push(LinkedList* list, void* value);

void* ll_get(const LinkedList* list, int index);

void ll_free(LinkedList* ptr);

void ll_print(const LinkedList* list);

#endif