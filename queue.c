/**
 * CS 537 Programming Assignment 2 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 10/13/2020
 * @brief Thread-safe queue implementation from proj2 with statistics 
 * functions removed.
 * @file queue.c
 */

#include "queue.h"

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Queue* q_initialize(const int size) {
    assert(size > 0);
    // Malloc the queue structure
    Queue* q = (Queue*)malloc(sizeof(Queue) + size * sizeof(void*));
    if (q == NULL) {
        perror("Error allocating memory for new Queue structure.\n");
        exit(EXIT_FAILURE);
    }

    // Setup queue spots and internal fields
    for (int i = 0; i < size; i++) {
        q->item[i] = NULL;
    }

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

void q_enqueue(Queue* q, void* value) {
    if (q == NULL) {
        perror("Can't enqueue an item to NULL.");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&q->lock);

    // WAIT UNTIL SPACE IF NECESSARY
    while (q->tail == (q->head + 1) % (q->size))
        pthread_cond_wait(&q->full, &q->lock);

    // write at index head
    assert(q->item[q->head] == NULL);
    q->item[q->head] = value;

    // advance head ptr
    q->head = (q->head + 1) % q->size;

    pthread_cond_signal(&q->empty);
    pthread_mutex_unlock(&q->lock);
}

void* q_dequeue(Queue* q) {
    if (q == NULL) {
        perror("Can't dequeue an item from NULL.");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&q->lock);

    // WAIT UNTIL VALUE IF NECESSARY
    while (q->tail == q->head) pthread_cond_wait(&q->empty, &q->lock);

    char* value = q->item[q->tail];    // retrieve from queue
    q->item[q->tail] = NULL;            // null out old ptr
    q->tail = (q->tail + 1) % q->size;  // advance tail ptr

    pthread_cond_signal(&q->full);
    pthread_mutex_unlock(&q->lock);

    return value;
}

void* q_peek(Queue* q) {
    if (q == NULL) {
        perror("NULL is not a valid queue.");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&q->lock);

    // WAIT UNTIL VALUE IF NECESSARY
    while (q->tail == q->head) pthread_cond_wait(&q->empty, &q->lock);

    char* value = q->item[q->tail];    // next in line

    pthread_mutex_unlock(&q->lock);

    return value;
}