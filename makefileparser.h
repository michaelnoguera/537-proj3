/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file makefileparser.h
 */


#ifndef _MAKEFILE_PARSER
#define _MAKEFILE_PARSER

#include <stdio.h>

#include "makefilerule.h"
#include "graph.h"

int ParseMakefile(FILE* makefile, Graph* graph);
#endif