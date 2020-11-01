/**
 * CS 537 Programming Assignment 2 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 10/13/2020
 * @file queue.h
 */

#ifndef _QUEUE_
#define _QUEUE_

#include <pthread.h>
#include <time.h>

/** Thread-safe implementation of a dynamically-allocated array-based queue
 */
typedef struct Queue {
    pthread_mutex_t lock;
    pthread_cond_t empty;
    pthread_cond_t full;

    int head;     /// next index to enqueue at
    int tail;     /// next index to dequeue from
    size_t size;  /// max items the queue can hold
    void* item[];
} Queue;

/**
 * Initializes a new empty Queue.
 * 
 * @param size queue capacity
 * 
 * @return pointer to new heap-allocated Queue
 */
Queue* q_initialize(const size_t size);

/**
 * Adds a new Node to the end of a Queue.
 * 
 * @param q The queue to enqueue to.
 * @param value The value of the new node, of type void*.
 */
void q_enqueue(Queue *q, void *value);

/**
 * Removes a node from the end of a queue
 * 
 * @param q The queue from which to dequeue from
 * @return the pointer that was removed, which can point to NULL
 */
void* q_dequeue(Queue *q);

/**
 * Peek at next node to be dequeued
 * 
 * @param q the queue, which will not be altered
 * @return the next value that would be returned by dequeue, without actually
 * removing it from the queue
 */
void* q_peek(Queue* q);

#endif
