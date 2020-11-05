/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file exec.h
 * @brief handles running the makefile commands given a sorted list
 */

#ifndef _EXEC_
#define _EXEC_

#include "bintree.h"
#include "linkedlist.h"
#include "makefilerule.h"
#include <sys/time.h>

/**
 * Executes each rule in the job list
 *
 * @param order Topologically sorted list of Rules to run
 * @param map Binary tree to search against to get rules from their target
 * names.
 */
int execRules(LinkedList* order, BTree* map);

#endif