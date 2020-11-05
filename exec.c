/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera (noguera) <mnoguera(at)wisc.edu>
 * @author Julien de Castelnau (de-castelnau) <decastelnau(at)wisc.edu>
 * @date 11/4/2020
 * @file exec.c
 * @brief Executes each rule as needed using fork and execvp
 */

#include "exec.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Execute the given command using fork() and execvp()
 * @details wrapper around bt_get that type casts for Rules
 * @param command command to execute
 * @return status value of the child process
 */
static int execCommand(char* command_string) {

    printf("\x1B[90mrunning \"%s\"\x1B[0m\n", command_string);
    Command* command = newCommandFromString(command_string);
    if (command == NULL) {
        fprintf(stderr, "ERROR: Could not parse command for rule");
        exit(EXIT_FAILURE);
    }

    int input_fd = -1;
    int output_fd = -1;

    if (command->inputfile != NULL) {
        input_fd = open(command->inputfile, O_RDONLY);
        if (input_fd == -1) {
            fprintf(stderr, "Error calling open() on infile");
            exit(EXIT_FAILURE);
        }
    }
    if (command->outputfile != NULL) {
        output_fd = open(command->outputfile, O_WRONLY | O_TRUNC | O_CREAT,
                         S_IRUSR | S_IWUSR);
        if (output_fd == -1) {
            fprintf(stderr, "Error calling open() for writing to outfile");
            exit(EXIT_FAILURE);
        }
    }

    pid_t child_pid;
    int status = 1; // default to no error

    child_pid = fork();
    if (child_pid < 0) {
        // there was an error calling fork
        perror("Error calling fork()");
        exit(EXIT_FAILURE);
    } else if (child_pid == 0) {
        // === CHILD PROCESS ===
        // Copy over file descriptors to redirect output
        if (input_fd != -1) dup2(input_fd, 0);   // 0 represents stdin
        if (output_fd != -1) dup2(output_fd, 1); // 1 represents stdout

        // use execvp() to start new command
        if (execvp(*command->argv, command->argv) < 0) { // execute the command
            perror("Error calling exec");
            exit(EXIT_FAILURE);
        }

        // run to completion
    } else {
        // === PARENT PROCESS ===
        // wait for child process to finish
        wait(&status); // wait for completion
    }
    return status;
}

/**
 * Helper method to get the modification time on a file
 * @param filename Name of file
 * @return file modification time, or 0 if not found
 */
static time_t getModDate(char* filename) {
    // create file descriptor for specified file
    int fd = open(filename, O_RDONLY);

    if (fd == -1) { // file doesn't exist, or can't be read, in which case it
                    // functionally doesn't exist
        return 0;
    }
    struct stat filestats;

    if (fstat(fd, &filestats) != 0) {
        return 0;
    } else {
        return filestats.st_mtime;
    }
}

/**
 * Helper method to search map for a key
 * @details wrapper around bt_get that type casts for Rules
 * @param map structure to search
 * @param key string key to search for
 * @return pointer to Rule with the name `key`
 */
static Rule* getRuleFromKey(BTree* map, char* key) {
    return (Rule*)bt_get(map, key);
}

/**
 * called for each entry in topoTodoList
 * check the rule and its dependencies to see if it's out of date, if so execute
 * commands
 * @param map structure to verify if deps are rules or files
 * @param rule Rule to execute commands for
 */
static void execRule(BTree* map, Rule* rule) {
    // guaranteed dependencies are already complete
    // printf("\x1B[32mexecuting target %s\x1B[0m\n", rule->target);

    bool outOfDate = false;

    time_t targetTime = getModDate(rule->target);
    if (targetTime == 0) outOfDate = true;

    int i = 0;
    char* depname;

    while (!outOfDate && i < rule->numdeps) {
        depname = rule->dependencies[i];
        time_t depTime = getModDate(depname);

        // No file was found. Assume it's a Rule?
        if (depTime == 0) {
            Rule* result = getRuleFromKey(map, depname);
            // Not found? Then it cannot be satisfied
            if (result == NULL) {
                fprintf(stderr,
                        "\x1B[91mERROR: No rule to make target '%s'\x1B[0m\n",
                        depname);
                exit(EXIT_FAILURE);
            }
            // If it was found, its timestamp is represented by 0, which is
            // already its value.
        }
        if (depTime == 0 || depTime > targetTime) outOfDate = true;
        i++;
    }

    if (outOfDate) {
        // then run command
        // get from linked list, unwrap, and call
        LinkedListNode* curr_command = (rule->commands)->head;
        for (int i = 0; i < rule->commands->size; i++) {
            if (execCommand(curr_command->value) != 0) {
                fprintf(stderr,
                        "\x1B[91mERROR: Command exited with non-zero return "
                        "value, stopping.\x1B[0m\n");
                exit(EXIT_FAILURE);
            }
            curr_command = curr_command->next;
        }
    }
}

/**
 * Run all rules in a topologically ordered list.
 *
 * @param order topologically ordered list of Rules
 * @param map that correlates target names with their Rules
 *
 * @return 0 upon success
 */
int execRules(LinkedList* order, BTree* map) {
    LinkedListNode* curr_rule = order->head;
    for (int i = 0; i < order->size; i++) {
        execRule(map, (Rule*)curr_rule->value);
        curr_rule = curr_rule->next;
    }
    return 0;
}