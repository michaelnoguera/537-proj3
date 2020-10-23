/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file makefileparser.h
 */

#include "makefilerule.h"

// node type?

// interpret
// "537ps:  537ps.o readproc.o parseopts.o output.o"
// into target name and dep names

char* getline(); // follows the rules

Rule* getRule(); // reads lines and structures information

char* nextCommand(Rule r);

// TODO read Makefile line by line according to rules

// TODO read in a whole rule and return
// - target
// - dependency
// - steps

// TODO do we need a runnable-line representation?
// TODO do we need a rule representaiton