#include "linkedlist.h"
#include "bintree.h"
#include "graph.h"

// makeTODOLIST:
    // iterates the topo order list: get next rule
        // FOR THIS RULE:
            // need to build = false
            // for each dependency, if dep is newer than target
                // need to build = true; break

            // 


// TODO take a string as input

// TODO fork and run the string

// STATIC FRIENDLY WRAPPER OVER EXECVP
// - take arguments
// - stdin
// - stdio
// 

// execCommand(char* command) {
    // for each command, fork

        // if you're the fork
        // CALL FRIENDLY EXECVP

        // if you're not:
            // use wait3 to wait on the fork to finish
            // capture
            // coalesce with forked proc
        
    // If it succeeded, set timestamp to now.
    // (r->timestamp)
// }

///@return file modification time, or 0 if not found
time_t getModDate(char* filename) {
    // construct filepath from pid
    char* filepath;
    if (asprintf(&filepath, "/proc/%d/status", pid) == -1) {
        printf("Error allocating memory to hold filepath for process number %d\n", pid);
        exit(EXIT_FAILURE);
    }

    // create statusfile input stream
    FILE* statusfile = fopen(filepath, "r");
    if (statusfile == NULL) {
        return 0;
    }
    free(filepath);  //upon success, filepath is no longer needed
    return date;
}

// called for each entry in topoTodoList
execRule (Rule* rule) {
    // guaranteed dependencies are already complete

    bool outOfDate = FALSE;

    time_t targetTime = getModDate(rule->target);
    if (targetTime == NULL) outOfDate = TRUE;

    int i = 0;
    while (!outOfDate) {
        depname = ll_get(rule->dependencies, i);
        // if file with depname doesn't exist:
            // No? Search BST for dependency?
            // Was something found? It's a target.
            // Was nothing found? error, it was a file but not found.

        time_T depTime = getModDate(depname);
        if (deptime == NULL || deptime > targetTime) outOfDate = TRUE;
        i++;
    }
    
    if (outOfDate) {
        for (command) {
            execCommand(command);
        }
    }
    

    // WE DO NEED TO BUILD

    // No? Search BST for dependency?
        // Was something found? It's a target.
        // Was nothing found? error, it was a file but not found.

    //if (!timestamp || dep->timestamp > timestamp) 
        // else return

    //    build this thing:
    //      for each command; execCommand(Rule);
   
}

/**
 * Helper method to search map for a key
 * @details wrapper around bt_get that type casts for Rules
 * @param map structure to search
 * @param key string key to search for
 * @return pointer to Rule with the name `key`
 */ 
static Rule* getRuleFromKey(BTree map, char* key) {
    return (Rule*)bt_get(map, key);
}

static LinkedListNode* iterator = NULL;
static Rule* nextRule(LinkedList order) {
    // get next in list, based on iterator
    // update iterator
}

// MAIN FUNCTION IN THIS MODULE:
int doStuff(LinkedList order, BTree map) {


    //
    return status;
}

["ls -lah", "cd ..", "echo 'test'"]
