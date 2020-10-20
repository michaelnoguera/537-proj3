/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file makefilerule.h
 */

typedef struct makefile_rule_t {
    char* target;
    char** dependencies;
    char** commands;
} Rule;

Rule* initializeMakefileRule()

char** getDependencies(rule);

char** getCommand(int index)