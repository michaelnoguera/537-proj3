/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 10/13/2020
 *
 * @file queue.c
 * @brief Thread-safe queue implementation that holds void pointers
 * @version 2.0
 *
 * @details This is our queue module from project 2, altered as follows:
 *  - values are now void pointers, to let this work as a quasi-generic type
 *  - the statistics module has been fully removed
 *  - A peek function was added, so that threads could act preemptively based on
 *    the next value they are going to recieve without committing to storing it.
 */

#include "queue.h"

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Initializes a new empty Queue.
 *
 * @param size queue capacity
 * @return pointer to new heap-allocated Queue
 */
Queue* q_initialize(const size_t size) {
    assert(size > 0);
    // Malloc the queue structure
    Queue* q = (Queue*)malloc(sizeof(Queue) + size * sizeof(void*));
    if (q == NULL) {
        perror("Error allocating memory for new Queue structure.\n");
        exit(EXIT_FAILURE);
    }

    // Setup queue spots and internal fields
    for (size_t i = 0; i < size; i++) { q->item[i] = NULL; }

    q->head = 0;
    q->tail = 0;
    q->size = size;

    // Setup synchronization
    if (pthread_mutex_init(&q->lock, NULL) != 0) {
        perror("Error initializing Queue mutex lock.\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&q->empty, NULL) != 0) {
        perror("Error initializing 'queue empty' condition variable.\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_cond_init(&q->full, NULL) != 0) {
        perror("Error initializing 'queue full' condition variable.\n");
        exit(EXIT_FAILURE);
    }

    return q;
}

/**
 * Adds a new Node to the end of a Queue.
 *
 * @param q The queue to enqueue to.
 * @param value The value of the new node, of type void*.
 */
void q_enqueue(Queue* q, void* value) {
    if (q == NULL) {
        perror("Can't enqueue an item to NULL.");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&q->lock);

    // WAIT UNTIL SPACE IF NECESSARY
    while (q->tail == (q->head + 1) % ((int)q->size))
        pthread_cond_wait(&q->full, &q->lock);

    // write at index head
    assert(q->item[q->head] == NULL);
    q->item[q->head] = value;

    // advance head ptr
    q->head = (q->head + 1) % q->size;

    pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
}

/**
 * Removes a node from the end of a queue
 *
 * @param q The queue from which to dequeue from
 * @return the pointer that was removed, which can point to NULL
 */
void* q_dequeue(Queue* q) {
    if (q == NULL) {
        perror("Can't dequeue an item from NULL.");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&q->lock);

    // WAIT UNTIL VALUE IF NECESSARY
    while (q->tail == q->head) pthread_cond_wait(&q->empty, &q->lock);

    char* value      = q->item[q->tail];        // retrieve from queue
    q->item[q->tail] = NULL;                    // null out old ptr
    q->tail          = (q->tail + 1) % q->size; // advance tail ptr

    pthread_cond_signal(&q->full);
    pthread_mutex_unlock(&q->lock);

    return value;
}

/**
 * Peek at next node to be dequeued
 *
 * @param q the queue, which will not be altered
 * @return the next value that would be returned by dequeue, without actually
 * removing it from the queue
 */
void* q_peek(Queue* q) {
    if (q == NULL) {
        perror("NULL is not a valid queue.");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&q->lock);

    // WAIT UNTIL VALUE IF NECESSARY
    while (q->tail == q->head) pthread_cond_wait(&q->empty, &q->lock);

    char* value = q->item[q->tail]; // next in line

    pthread_mutex_unlock(&q->lock);

    return value;
}