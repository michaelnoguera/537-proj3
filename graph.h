/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Julien de Castelnau and Michael Noguera 
 * @date 11/4/2020
 * @brief Implementation of a directed dependency graph of makefile rules using a linkedlist and a String-keyed BST (See bintree.h).
 * @file graph.h
 */

#ifndef _GRAPH_
#define _GRAPH_
#include "bintree.h"
#include "makefilerule.h"
#include "linkedlist.h"

enum Status {VISITED, VISITING, UNVISITED};

typedef struct g_Graph_t {
    int size; 
    LinkedList* /*of g_Node_t*s */ nodes;
    BTree* searchtree; 
} Graph;

typedef struct g_Node_t {
    enum Status mark;
    Rule* contents;
} GNode;

/**
 * Initialize a new graph, and initialize all of its constituent data structures (LinkedList, BTree) as well.
 * 
 * @param none
 * 
 * @return A pointer to the new graph, or NULL if an error occurred.
 */
Graph* initGraph();

/** 
 * Initialize a graph node, adding it to the graph's internal linked list and indexing it within the binary search tree.
 * 
 * @param g Graph to add to.
 * @param contents A makefile rule that the new node should hold.
 * 
 * @return A pointer to the new node created, or NULL if an error occurred.
 */
GNode* initializeGraphNode(Graph* g, Rule* contents);

/**
 * Sorts the specified graph topologically, i.e. returns an ordered linked list of the nodes based on which nodes depend on each other.
 * 
 * @param g Graph to sort.
 * 
 * @return A LinkedList, if no circular dependency is found in visit().
 */
LinkedList* topologicalSort(Graph* g);

/**
 * Performs a topological sort within a subgraph of g
 * 
 * @param[in] g graph containing makefile rule
 * @param[in] NodeStringKey target to build
 * @return LinkedList containing topological ordering or NULL upon target not 
 * found
 */
LinkedList* topologicalSortFromNode(Graph* g, char* NodeStringKey);
#endif