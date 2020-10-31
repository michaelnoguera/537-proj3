
/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Julien de Castelnau and Michael Noguera 
 * @date 11/4/2020
 * @brief Implementation of a directed dependency graph of makefile rules using a linkedlist and a String-keyed BST (See bintree.h).
 * @file graph.c
 */

#include "linkedlist.h"
#include "graph.h"

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>


// Graph constructor
Graph* initGraph() {
    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (g == NULL) {
        perror("Error allocating memory for new graph.\n");
        exit(EXIT_FAILURE);
    }
    g->size = 0;

    if ((g->nodes = ll_initialize()) == NULL) {
        perror("Error initializing linked list for new graph.");
        exit(EXIT_FAILURE);
    }

    if ((g->searchtree = bt_initializeTree()) == NULL) {
        perror("Error initializing search tree for graph.");
        exit(EXIT_FAILURE);
    }

    return g;
}

// Graph node initializer.
Node* initializeGraphNode(Graph* g, Rule* contents) {
    if (g == NULL) {
        perror("Can't add a graph node to NULL.");
        exit(EXIT_FAILURE);
    }

    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("Failed to allocate memory for new graph node.");
        exit(EXIT_FAILURE);
    }

    new_node->contents = contents;
    new_node->mark = UNVISITED;

    assert(g->nodes != NULL);
    assert(g->searchtree != NULL);

    bt_insert(g->searchtree, (new_node->contents)->target, new_node); // Add to BST
    ll_push(g->nodes, new_node);
    g->size++;

    return new_node;
}

/**
 * Helper method for topological sort, traverses through nodes and their dependencies.
 * 
 * @param searchTree BTree to search dep names against 
 * @param l LinkedList to add results to
 * @param n Currently visited node
 * 
 */
void visit(BTree* searchTree, LinkedList* l, Node* n) {
    assert(l != NULL);
    assert(n != NULL);

    if (n->mark == VISITED) return;
    if (n->mark == VISITING) {
        fprintf(stderr, "ERROR: Circular dependency detected\n");
        exit(EXIT_FAILURE);
    }

    n->mark = VISITING;

    char** successors = n->contents->dependencies;
    for (int i = 0; i < n->contents->numdeps; i++) {

        // Some elements of successors will map to NULL, this corresponds to
        // spots where the dependencies are assumed to be file targets
        // (i.e. not declared in the actual makefile.)

        Node* search_result = (Node*)bt_get(searchTree, successors[i]);
        
        if (search_result != NULL) {
            visit(searchTree, l, search_result);
        }
    }

    n->mark = VISITED;
    ll_push(l, n->contents);
}

// Topological sort
LinkedList* topologicalSort(Graph* g) {
    assert(g != NULL);

    LinkedList* sorted;

    if ((sorted = ll_initialize()) == NULL) {
        perror("Error initializing linked list for new graph.");
        exit(EXIT_FAILURE);
    }

    struct ll_node_t* curr = g->nodes->head;
    for (int i = 0; i < g->size; i++) {
        if (((Node*)curr->value)->mark == UNVISITED) {
            visit(g->searchtree, sorted, ((Node*)curr->value));
        }
        curr = curr->next;
    }

    return sorted;
}