/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file main.c
 */

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "graph.h"
#include "exec.h"
#include "makefileparser.h"
#include "makefilerule.h"

FILE* openMakefile(char* filename) {
    FILE* makefile;
    if (filename != NULL) {
        makefile = fopen(filename, "r");
        if (makefile != NULL) return makefile;
        fprintf(stderr,"ERROR: Invalid makefile specified.\n");
        exit(EXIT_FAILURE);
    } else {
        makefile=fopen("makefile", "r");
        if (makefile != NULL) return makefile;
        makefile = fopen("Makefile", "r");
        if (makefile != NULL) return makefile;
        fprintf(stderr,"ERROR: No makefile found and it was not \
                        specified on the command line.\n");
        exit(EXIT_FAILURE);
        
    }
}

int main(int argc, char** argv) {
    int opt = 0;
    char* filename = NULL;
    char* targetname = NULL;

    while ((opt = getopt(argc, argv, "-f:")) != -1) {
        switch ((char)opt) {
            case 'f':
                filename = optarg;
                break;
            case '?':
                fprintf(stderr, "ERROR: error parsing command line args");
                exit(EXIT_FAILURE);
                break;
            default:
                targetname = optarg;
                break;
        }
    }

    FILE* makefile = openMakefile(filename);

    Graph* g = ParseMakefile(makefile);
    BTree* map = g->searchtree;
    bt_print(map);

    LinkedList* ordering;

    if (targetname == NULL) {
        GNode* first_target = ll_get(g->nodes, 0);
        if (first_target == NULL) {
            fprintf(stderr, "ERROR: No targets in makefile.\n");
            exit(EXIT_FAILURE);
        } else {
            ordering = topologicalSortFromNode(g, first_target);
        }
    } else {
        GNode* result = (GNode*)bt_get(g->searchtree, targetname);
        if (result == NULL) {
            fprintf(stderr, "ERROR: '%s' is not a valid target.\n", targetname);
            exit(EXIT_FAILURE);
        } else {
            ordering = topologicalSortFromNode(g, result);
        }
    }

    execRules(ordering, map);

    return EXIT_SUCCESS;
}