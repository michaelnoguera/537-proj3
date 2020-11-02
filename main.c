/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file main.c
 */

#include <stdio.h>

#include "graph.h"
#include "exec.h"
#include "makefileparser.h"

int main(int argc, char** argv) {
    FILE* makefile = fopen("makefile", "r");
    Graph* g = ParseMakefile(makefile);
    BTree* map = g->searchtree;
    LinkedList* ordering = topologicalSortFromNode(g, argv[1]);

    execRules(ordering, map);

    return EXIT_SUCCESS;
}