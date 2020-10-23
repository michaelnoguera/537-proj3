/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file main.c
 */

#include "graph.h"
#include <stdio.h>

// 1. TODO parse args
// --> TODO handle stdin/stdout

// 2. TODO

// Basic graph testing program in place for now

// Equivalent makefile:

/*

A: B C
    touch A

B: C D
    touch B

C: D E
    touch C

D: 
    touch D
*/


int main() {
    Graph* g;
    
    if ((g = initGraph()) == NULL) {
        perror("Graph creation failed.");
        exit(EXIT_FAILURE);
    }

    // A test based on the sample makefile from above

    char* a[] = {"B", "C"};
    char* b[] = {"C", "D"};
    char* c[] = {"D","E"};

    Rule* r1 = (Rule*) malloc(sizeof(Rule));
    r1->target = "A";
    r1->dependencies = a;
    r1->numdeps = 2;

    Rule* r2 = (Rule*) malloc(sizeof(Rule));
    r2->target = "B";
    r2->dependencies = b;
    r2->numdeps = 2;

    Rule* r3 = (Rule*) malloc(sizeof(Rule));
    r3->target = "C";
    r3->dependencies = c;
    r3->numdeps = 3;

    Rule* r4 = (Rule*) malloc(sizeof(Rule));
    r4->target = "D";
    r4->dependencies = NULL;
    r4->numdeps = 0;

    initializeGraphNode(g, r1);
    initializeGraphNode(g, r2);
    initializeGraphNode(g, r3);
    initializeGraphNode(g, r4);

    LinkedList* l = topologicalSort(g);

    LinkedListNode* curr = l->head;

    for (int i = 0; i < l->size; i++) {
        printf("%s, ", (((Rule*)curr->value)->target));
        curr = curr->next;
    }
    printf("\n");
    
    return 0;
}