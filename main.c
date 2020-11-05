/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file main.c
 */

#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "exec.h"
#include "graph.h"
#include "makefileparser.h"
#include "makefilerule.h"

/**
 * Opens the requested makefile, or defaults to "makefile" (then "Makefile"),
 * while handling errors.
 *
 * @param filename makefile
 *
 * @return FILE* to the makefile, in read mode
 */
FILE* openMakefile(char* filename) {
    FILE* makefile;
    if (filename != NULL) {
        makefile = fopen(filename, "r");
        if (makefile != NULL) return makefile;
        fprintf(stderr, "ERROR: Invalid makefile specified.\n");
        exit(EXIT_FAILURE);
    } else {
        makefile = fopen("makefile", "r");
        if (makefile != NULL) return makefile;
        makefile = fopen("Makefile", "r");
        if (makefile != NULL) return makefile;
        fprintf(stderr,
                "ERROR: No makefile found and it was not \
                        specified on the command line.\n");
        exit(EXIT_FAILURE);
    }
}

// Main function, gets input and coordinates program execution.
int main(int argc, char** argv) {
    int opt = 0;
    char* filename = NULL;
    char* targetname = NULL;

    // 0. User can specify makefile using -f flag (extra credit)
    while ((opt = getopt(argc, argv, "-f:")) != -1) {
        switch ((char)opt) {
            case 'f':
                filename = optarg;
                break;
            case '?':
                fprintf(stderr, "ERROR: error parsing command line args\n");
                fprintf(stderr, "Valid usages are:\n");
                fprintf(stderr, "\t./537make\n");
                fprintf(stderr, "\t./537make TARGET\n");
                fprintf(stderr, "\t./537make -f MAKEFILE\n");
                fprintf(stderr, "\t./537make -f MAKEFILE TARGET\n");
                fprintf(
                  stderr,
                  "If more than one target is specified, the last is used.\n");
                exit(EXIT_FAILURE);
                break;
            default:
                targetname = optarg;
                break;
        }
    }

    // 1. Get the makefile
    FILE* makefile = openMakefile(filename);

    // 2. Parse the makefile and construct a binary tree map
    Graph* g = ParseMakefile(makefile);
    BTree* map = g->searchtree;

    // 3. Topological ordering and some validation
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

    // 4. Run the rules in order
    execRules(ordering, map);

    return EXIT_SUCCESS;
}