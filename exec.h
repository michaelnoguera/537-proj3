/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file exec.h
 */

#ifndef _EXEC_
#define _EXEC_

#include <sys/time.h>
#include "linkedlist.h"
#include "bintree.h"
#include "makefilerule.h"

/**
 * Execute the given command using fork() and execvp()
 * @details wrapper around bt_get that type casts for Rules
 * @param command command to execute
 * @return status value of the child process
 */ 
int execCommand(char* command);

/**
 * Helper method to get the modification time on a file
 * @param filename Name of file
 * @return file modification time, or 0 if not found
 */ 
time_t getModDate(char* filename);

/**
 * called for each entry in topoTodoList
 * check the rule and its dependencies to see if it's out of date, if so execute commands
 * @param map structure to verify if deps are rules or files
 * @param rule Rule to execute commands for
 */
void execRule (BTree* map, Rule* rule);

/**
 * MAIN FUNCTION IN THIS MODULE; executes each rule in the job list
 * @param order Topologically sorted list of Rules
 * @param map Binary tree to search against for dep verification
 */
int execRules(LinkedList* order, BTree* map);
#endif