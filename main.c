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
#include "makefilerule.h"

int main(int argc, char** argv) {
    /*FILE* makefile;
    makefile=fopen("makefile", "r");
    if (makefile==NULL) { 
        makefile = fopen("Makefile", "r");
        if (makefile==NULL) {
            fprintf(stderr,"ERROR: No makefile found and it was not specified on the command line.\n");
            exit(EXIT_FAILURE);
        }
    }
    Graph* g = ParseMakefile(makefile);
    BTree* map = g->searchtree;
    bt_print(map);

    LinkedList* ordering;

    if (argc <= 1) {
        GNode* first_target = ll_get(g->nodes, 0);
        if (first_target == NULL) {
            fprintf(stderr, "ERROR: No targets specified in makefile.\n");
            exit(EXIT_FAILURE);
        } else {
            ordering = topologicalSortFromNode(g, first_target);
        }
    } else {
        GNode* result = (GNode*)bt_get(g->searchtree, argv[1]);
        if (result == NULL) {
            fprintf(stderr, "ERROR: '%s' is not a valid target.\n", argv[1]);
            exit(EXIT_FAILURE);
        } else {
            ordering = topologicalSortFromNode(g, result);
        }
    }*/

    //newCommandFromString("one two three four five");
    //Command* c = newCommandFromString("ls -l -a -h");
    //execCommand(c);
    //newCommandFromString("hello world < in > out");
    //newCommandFromString("one two three four five   ");

    //execRules(ordering, map);
    execCommand("wc > out < in");

    return EXIT_SUCCESS;
}