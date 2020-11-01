/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @brief Representation of a makefile rule stucture.
 * @file makefilerule.h
 */

#ifndef _MAKEFILE_RULE
#define _MAKEFILE_RULE

#include "linkedlist.h"
#include "time.h"

typedef struct makefile_rule_t {
    char* target;
    char** dependencies;
    int numdeps;
    LinkedList* commands;
    int linenumber;
} Rule;

typedef struct makefile_command_t {
    char** argv;
    char* input;
    char* output;
    int linenumber;
} Command;

// Will want initializeMakefileRule but the others aren't used at the moment. Thoughts?
/*Rule* initializeMakefileRule()

char** getDependencies(Rule* rule);

char** getCommand(int index)*/
#endif