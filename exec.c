#include "exec.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdbool.h>
#include <stdio.h>

//TODO: properly implement return value
int execCommand(Command* command) {
    pid_t child_pid;
    int status;
    
    child_pid = fork();
    if(child_pid < 0) {
        // there was an error calling fork
        perror("Error calling fork()");
        exit(EXIT_FAILURE);
    } else if(child_pid == 0) {
        // CHILD PROCESS
        // use execvp() to start new command
        printf("command: %s\n", *command->argv);
        if (execvp(*command->argv, command->argv) < 0) {     /* execute the command  */
               perror("Error calling exec");
               exit(1);
        }
    } else {
        // PARENT PROCESS
        // wait for child process to finish
        while (wait(&status) != child_pid)       /* wait for completion  */
            ;
    }
    return status;
}

time_t getModDate(char* filename) {
    // create file descriptor for specified file
    int fd = open(filename, O_RDONLY);

    // TODO: add error handling?
    // if the return value is -1 it could be because the file doesn't exist, or because some other
    // error occurred. better error handling is probably in order
    if (fd == -1) {
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

void execRule (BTree* map, Rule* rule) {
    // guaranteed dependencies are already complete

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
                fprintf(stderr, "ERROR: No rule to make target '%s'\n", depname);
                exit(EXIT_FAILURE);
            }
            // If it was found, its timestamp is represented by 0, which is already its value.
        }
        if (depTime == 0 || depTime > targetTime) outOfDate = true;
        i++;
    }
    
    if (outOfDate) {
        LinkedListNode* curr_command = (rule->commands)->head;
        for (int i = 0; i < rule->commands->size; i++) {
            execCommand((Command*)curr_command->value);
            curr_command = curr_command->next;
        }
    }
}

int execRules(LinkedList* order, BTree* map) {
    LinkedListNode* curr_rule = order->head;
    for (int i = 0; i < order->size; i++) {
        execRule(map, (Rule*)curr_rule->value);
        curr_rule = curr_rule->next;
    }
    return 0;
}