/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file makefileparser.h
 * @brief Multithreaded makefile parsing module that follows assignment rules
 */


#ifndef _MAKEFILE_PARSER
#define _MAKEFILE_PARSER

#include <stdio.h>

#include "graph.h"
#include "makefilerule.h"

/**
 * Parses a makefile into its dependency graph representation
 *
 * @param makefile file to parse
 * @return Graph* of makefile Rules that contains all entries from the makefile,
 * or exits gracefully upon error.
 *
 * @example
 * ```C
 * FILE* makefile = fopen("test/simple_testcase/makefile", "r");
 * Graph* g = ParseMakefile(makefile);
 * ```
 */
Graph* ParseMakefile(FILE* makefile);
#endif