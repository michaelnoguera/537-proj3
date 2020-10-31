/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera
 * @date 11/4/2020
 * @brief Parses a makefile according to the assignment rules and constructs
 * MakefileRule representations for each target
 * @file makefileparser.c
 */
#define _GNU_SOURCE

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <semaphore.h>

#include "makefileparser.h"
#include "queue.h"
#include "graph.h"
#include "makefilerule.h"

static void printerr(int linenum, char* msg, char* badline) {
    fprintf(stderr, "%i: <%s>: %s\n", linenum, msg, badline);
}

/**
 * Custom readline() implementation that meets the assignment requirements.
 * Reads from specified file.
 * 
 * @param[in] fp, pointer to the FILE to read from
 * @param[out] overflow flag to set if input size greater than buffer
 * @param[out] endOfFile flag to set if EOF reached
 * 
 * @return pointer to heap-allocated string on success, NULL and the appropriate
 * flag on failure
 */
static char* readerReadLine(FILE* fp, bool* overflow, bool* endOfFile) {
    char c;
    unsigned int i = 0;

    // allocate a buffer to hold the line
    const size_t BUFSIZE = 4096;
    char* buf = (char*)calloc(BUFSIZE, sizeof(char));
    if (buf == NULL) {
        perror("Error allocating memory to hold line.");
        exit(EXIT_FAILURE);
    }

    // read input one character at at time
    while (i < BUFSIZE) {
        c = getchar();

        // CASE 1: EOF on newline or EOF left on buffer by CASE 2
        // -> return sentinel with EOF flag set
        if (i == 0 && c == EOF) {
            *endOfFile = true;
            free(buf);
            return NULL;
        }

        // CASE 2: current string ended, either by newline or EOF
        // -> return string
        // -> if EOF was reached, CASE 1 will be triggered upon next read
        if (c == EOF || c == '\n') {
            buf[i] = '\0';
            return buf;
        }

        // CASE 3: nothing special, store the character
        buf[i] = c;
        i++;
    }

    // OVERFLOW: line length exceeds BUFSIZE
    // -> set overflow flag
    // -> consume the remainder of the oversized line
    *overflow = true;
    buf[BUFSIZE - 1] = '\0';

    while (c != '\n' && c != EOF) {
        c = getchar();
    }

    return buf;  // return buffer, truncated to BUFSIZE characters
}

/**
 * Reader thread. Consumes input from stdin and parses it into the first
 * Muncher's queue.
 * 
 * @param outputQueue void* wrapping a Queue*, which is the queue where Reader
 *  should put strings it reads
 * 
 * @details does not close fp
 */
void* Reader(void* args) {
    FILE* fp = ((FILE**)args)[0];
    Queue* outputQueue = ((Queue**)args)[1];
    assert(fp != NULL && outputQueue != NULL);

    int linenum = 0;
    bool overflow = false;
    bool endOfFile = false;

    while (!endOfFile) {
        linenum++;
        overflow = false;
        char* line = readerReadLine(fp, &overflow, &endOfFile);

        if (overflow) {
            printerr(linenum, "line too long", line);
            continue;  // do not enqueue oversize line
        }

        q_enqueue(outputQueue, line);
    }

    // OUT OF LINES -> enqueue null sentinel value
    q_enqueue(outputQueue, NULL);

    pthread_exit(NULL);
}

static bool isCommandLine(const char* line) {
    assert(line != NULL);
    if isblank (line[0]) return false;  // target lines start in column 1
    char* colon = index(line, ':');
    if (!colon) return false;  // target lines must contain colons

    // if there's a comment, make sure the colon isn't part of it
    char* comment = strchr(line, '#');
    return (comment == NULL || colon < comment);
}

/**
 * Takes strings read as input and makes them into Rules.
 * 
 * @param args void* wrapping a Queue*[2], where `queue[0]` is used for input
 *  to this function and `queue[1]` is used for output.
 */
void* RuleConstructor(void* args) {
    Queue* in = ((Queue**)args)[0];
    Queue* out = ((Queue**)args)[1];
    assert(in != NULL && out != NULL);

    //TODO need to handle various kinds of whitespace
    char* line;
    //Rule* r = NULL;
    while ((line = q_dequeue(in)) != NULL) {
        if (isCommandLine(line)) {
            // get target
            char* colon = strchr(line, ':');
            int targetsize = colon - line;
            char* target = malloc(sizeof(char) * (targetsize + 1));
            if (target == NULL) {
                fprintf(stderr, "Error allocating memory for target name\n");
                exit(EXIT_FAILURE);
            }

            strncpy(line, target, targetsize);
            target[targetsize] = '\0';

            printf("%s\n", target);


            // strdependencies = strtok(strchr(line, ':')[1], "\t ")
        }
        //char* end = strchr(line, '#'); // pointer to end of meaningful data
        //if (end - line == 0) continue; // comment line
        //if (end == NULL) end = line+(strlen(line)-1)
        
        //char* c = line;
        //while (c < end) {
        //    if (c == ' ' || c == '\t') continue; // ignore whitespace
        //}

        // === 2. FIND ALL COMMANDS ===
        //while (1) { // while this rule still has more commands
         //   char* next = q_peek(in);
//
        //    if (next[0] != '\t') break;
        //    if (index(l))
//
         //   (line = q_dequeue(in)) != NULL
    }

    // enqueue null sentinel value
    q_enqueue(out, NULL);
    pthread_exit(NULL);
}

/**
 * Adds Rules to the specified Graph as they are recieved from inputQueue.
 * 
 * @param args arguments wrapped in a void*[2]: [0] is the `Queue* inputQueue`,
 * which delivers Rule*s to be added to the graph. [1] is the `Graph* graph`,
 * where the rules are to be stored
 * 
 * @return void* for thread closure
 */
void* Grapher(void* args) {
    Queue* in = ((Queue**)args)[0];  //TODO verify this doesn't lead to memory alignment errors
    Graph* graph = ((Graph**)args)[1];
    assert(in != NULL && graph != NULL);

    Rule* next;

    //TODO count number of dependencies

    // add each rule to the graph
    while ((next = (Rule*)q_dequeue(in)) != NULL)
        initializeGraphNode(graph, next);

    pthread_exit(NULL);  // return successfully
}

// TODO read Makefile line by line according to rules

// TODO read in a whole rule and return
// - target
// - dependency
// - steps

// TODO do we need a runnable-line representation?
// TODO do we need a rule representaiton

int ParseMakefile(FILE* makefile, Graph* graph) {
    pthread_t reader_id;
    pthread_t ruleConstructor_id;
    pthread_t grapher_id;

    const int QUEUE_SIZE = 25;
    Queue* readerToRuleConstructor = q_initialize(QUEUE_SIZE);
    Queue* ruleConstructorToGrapher = q_initialize(QUEUE_SIZE);

    // Spawn child threads
    void* reader_args[2] = {makefile, readerToRuleConstructor};
    if (pthread_create(&reader_id, NULL, Reader, reader_args) != 0) {
        perror("Error in Reader thread creation.");
        exit(EXIT_FAILURE);
    }

    // Workers take an array of {inputQueue, outputQueue} as parameters
    void* ruleConstructor_args[2] = {readerToRuleConstructor,
                                     ruleConstructorToGrapher};
    if (pthread_create(&ruleConstructor_id, NULL,
                       RuleConstructor, ruleConstructor_args) != 0) {
        perror("Error in Munch1 thread creation.");
        exit(EXIT_FAILURE);
    }

    void* grapher_args[2] = {ruleConstructorToGrapher, graph};
    if (pthread_create(&grapher_id, NULL, Grapher, grapher_args) != 0) {
        perror("Error in Writer thread creation.");
        exit(EXIT_FAILURE);
    }

    // Merge threads
    if (pthread_join(reader_id, NULL) != 0) {
        perror("Error in Reader thread termination.");
        exit(EXIT_FAILURE);
    }
    if (pthread_join(ruleConstructor_id, NULL) != 0) {
        perror("Error in Munch1 thread termination.");
        exit(EXIT_FAILURE);
    }
    if (pthread_join(grapher_id, NULL) != 0) {
        perror("Error in Munch2 thread termination.");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}