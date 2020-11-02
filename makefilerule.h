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

#include <stdio.h>

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
    char* executable;
    char** argv;
    char* inputfile;
    char* outputfile;
} Command;

/// Construts a new Rule struct with default values.
Rule* newRule();

/// Constructs a new Command struct with default values
Command* newCommand();

/// Constructs a new Command struct from a given makefile line
Command* newCommandFromString(char* s);

/// Prints out a text representation of a provided rule
void printMakefileRule(Rule* r);

#endif