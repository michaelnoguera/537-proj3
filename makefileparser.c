/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera
 * @date 11/4/2020
 * @brief Parses a makefile according to the assignment rules and constructs
 * MakefileRule representations for each target
 * @file makefileparser.c
 */
#define _GNU_SOURCE

#include "makefileparser.h"

#include <assert.h>
#include <ctype.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "graph.h"
#include "makefilerule.h"
#include "queue.h"

struct line {
    int linenum;
    char* string;
};

static const size_t MAX_LINE_LEN = 4096;


static void exitwitherr(int linenum, char* msg, char* badline) {
    fprintf(stderr, "%i: <%s>: %s\n", linenum, msg, badline);
    exit(EXIT_FAILURE);
}

/**
 * Custom readline() implementation that meets the assignment requirements.
 * Reads from specified file.
 * 
 * @param[in] fp, pointer to the FILE to read from
 * @param[out] overflow flag to set if input size greater than buffer
 * @param[out]
 * @param[out] endOfFile flag to set if EOF reached
 * 
 * @return pointer to heap-allocated string on success, NULL and the appropriate
 * flag on failure
 */
static char* readerReadLine(FILE* fp, bool* overflow, bool* nullchar, bool* endOfFile) {
    char c;
    unsigned int i = 0;

    // allocate a buffer to hold the line
    char* buf = (char*)calloc(MAX_LINE_LEN, sizeof(char));
    if (buf == NULL) {
        perror("Error allocating memory to hold line.\n");
        exit(EXIT_FAILURE);
    }

    // read input one character at at time
    while (i < MAX_LINE_LEN) {
        c = getc(fp);

        // CASE 1: EOF on newline or EOF left on buffer by CASE 3
        // -> return sentinel with EOF flag set
        if (i == 0 && c == EOF) {
            *endOfFile = true;
            free(buf);
            return NULL;
        }

        // CASE 2: catch invalid line errors
        if (c == '\0') {
            *nullchar = true;
            continue;
        }

        // CASE 3: current string ended, either by newline or EOF
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
    buf[MAX_LINE_LEN - 1] = '\0';

    while (c != '\n' && c != EOF) {
        c = getc(fp);
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
    bool nullchar = false;
    bool endOfFile = false;

    while (!endOfFile) {
        linenum++;
        overflow = false;
        nullchar = false;
        char* string = readerReadLine(fp, &overflow, &nullchar, &endOfFile);

        if (overflow) {
            exitwitherr(linenum, "line too long", string);
            // alternatively continue;  // do not enqueue oversize line
        }

        if (nullchar) {
            exitwitherr(linenum, "illegal null char in line", string);
            // alternatively continue;  // do not enqueue invalid line
        }

        printf("[READER] just read: %s\n", string);

        struct line* wrapper = malloc(sizeof(struct line));
        wrapper->linenum = linenum;
        wrapper->string = string;
        q_enqueue(outputQueue, wrapper);
    }

    // OUT OF LINES -> enqueue null sentinel value
    q_enqueue(outputQueue, NULL);

    pthread_exit(NULL);
}

static bool isTargetLine(const char* line) {
    assert(line != NULL);
    if (isblank(line[0])) return false;  // target lines start in column 1
    char* colon = index(line, ':');
    if (!colon) return false;  // target lines must contain colons

    // if there's a comment, make sure the colon isn't part of it
    char* comment = strchr(line, '#');
    return (comment == NULL || colon < comment);
}

static bool isCommandLine(const char* line) {
    assert(line != NULL);
    if (line[0] != '\t') return false;  // must start with tab

    size_t first_non_whitespace_char = strspn(line, "\t ");

    if (first_non_whitespace_char == strnlen(line, MAX_LINE_LEN)) {
        return false;  // entire line is whitespace
    } else if (line[first_non_whitespace_char] == '#') {
        return false;  // entire line is comment
    } else {
        return true;
    }
}

static bool isIgnoredLine(const char* line) {
    assert (line != NULL);
    size_t first_non_whitespace_char = strspn(line, "\t ");

    if (first_non_whitespace_char == strnlen(line, MAX_LINE_LEN)) return true;
    if (line[first_non_whitespace_char] == '#') return true;

    return false;
}

static Rule* newRule(int linenum) {
    Rule* rule = malloc(sizeof(Rule));

    rule->target = NULL;
    rule->numdeps = 0;
    rule->dependencies = NULL;
    rule->linenumber = linenum;
    rule->commands = ll_initialize();

    return rule;
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

    Rule* rule = NULL;
    struct line* wrapped;
    while ((wrapped = q_dequeue(in)) != NULL) {
        // UNWRAP
        char* line = wrapped->string;
        int linelen = strnlen(line, MAX_LINE_LEN);
        int linenum = wrapped->linenum;

        printf("[RULE CONSTRUCTOR] %s '%s'\n", "received", line);
        if (isTargetLine(line)) {
            if (rule != NULL) q_enqueue(out, rule);
            rule = newRule(linenum);

            printf("[RULE CONSTRUCTOR] line %i is a target\n", linenum);

            // GET TARGET ==> char* target
            char* target;  //target that will be built by this Rule
            char* colon;   // location of : delimiter denoting end of target name
            {
                colon = strchr(line, ':');
                size_t targetsize = colon - line;  // as a number of chars

                target = malloc(sizeof(char) * (targetsize + 1));
                if (target == NULL) {
                    exitwitherr(linenum, "Error allocating memory for target name\n", line);
                }

                strncpy(target, line, targetsize);
                target[targetsize] = '\0';
            }
            rule->target = target;
            printf("[RULE CONSTRUCTOR] ==> name '%s'\n", target);

            // GET DEPENDENCIES ==> char** dep_array
            int numdeps = 0;
            char** dependencies;
            {
                LinkedList* dep_ll = ll_initialize();

                const char dep_delimiters[] = "\t ";
                char* saveptr = NULL;
                char* dep = strtok_r(colon + 1, dep_delimiters, &saveptr);
                do {
                    assert(dep != colon);
                    printf("[RULE CONSTRUCTOR] --> dependency '%s'\n", dep);
                } while ((dep = strtok_r(NULL, dep_delimiters, &saveptr)) != NULL);

                numdeps = dep_ll->size;
                dependencies = (char**)ll_to_array(dep_ll);
                ll_print_strings(dep_ll);
                ll_destruct(dep_ll);  // free overhead while leaving pointees
            }
            rule->numdeps = numdeps;
            rule->dependencies = dependencies;
        } else if (isCommandLine(line)) {
            char* command;
            {
                size_t commandstart = strspn(line, "\t ");
                size_t commandsize = linelen-commandstart;  // as a number of chars
                printf("command start: %li, command size: %li\n", commandstart, commandsize);

                if (commandsize <= 0) {
                    exitwitherr(linenum, "command must have positive length", line);
                }
                command = malloc(sizeof(char) * (commandsize + 1));
                if (command == NULL) {
                    exitwitherr(linenum, "error allocating memory to hold line", line);
                }

                strncpy(command, line, commandsize);
                command[commandsize] = '\0';
            }
            ll_push(rule->commands, command);
        } else if (isIgnoredLine(line)) {
            printf("[RULE CONSTRUCTOR] (skipping empty line)\n");
        } else {
            exitwitherr(linenum, "line is invalid-not a target, command, comment, or blank", line);
        }

        free(line);
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

    //Rule* next;

    //TODO count number of dependencies

    // add each rule to the graph
    //while ((next = (Rule*)q_dequeue(in)) != NULL)
    //    initializeGraphNode(graph, next);

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