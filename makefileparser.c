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

// Wrapper struct to pass line numbers along with line contents
struct line {
    int linenum;
    char* string;
};

// Input buffer size, lines that exceed this length (including null terminator)
// result in errors.
static const size_t MAX_LINE_LEN = 4096;

/**
 * Helper func that prints formatted error messages
 * 
 * @param linenum line on which the error occurred
 * @param msg error message to display
 * @param badline pointer to bad line, to print
 */
static void exitwitherr(int linenum, char* msg, char* badline) {
    fprintf(stderr, "%i: <%s>: %s\n", linenum, msg, badline);
    exit(EXIT_FAILURE);
}

/**
 * Custom readline() implementation that meets the assignment requirements.
 * Reads from specified file.
 * 
 * @param[in] fp pointer to the FILE to read from
 * @param[out] overflow flag to set if input size greater than buffer
 * @param[out] nullchar flag to set if a null character is encountered in the 
 * middle of a line
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
 * Reader thread. Consumes input from stdin and wraps valid lines into line
 * structs.
 * 
 * @param args void* wrapping `[FILE* toOpen, Queue* toPutLinesIn]`
 * @return void* for thread closure
 * 
 * @details does not close fp
 */
static void* Reader(void* args) {
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

        if (endOfFile) break;
        if (string == NULL) exitwitherr(linenum, "error while reading input", string);
        if (overflow) exitwitherr(linenum, "line too long", string);
        if (nullchar) exitwitherr(linenum, "illegal null char in line", string);

        struct line* wrapper = malloc(sizeof(struct line));
        wrapper->linenum = linenum;
        wrapper->string = string;
        q_enqueue(outputQueue, wrapper);
    }

    // OUT OF LINES -> enqueue null sentinel value
    q_enqueue(outputQueue, NULL);

    pthread_exit(NULL);
}

/// Returns true if the given line is a valid target, or false otherwise.
static inline bool isTargetLine(const char* line) {
    assert(line != NULL);
    if (isblank(line[0])) return false;  // target lines start in column 1
    char* colon = index(line, ':');
    if (!colon) return false;  // target lines must contain colons

    // if there's a comment, make sure the colon isn't part of it
    char* comment = strchr(line, '#');
    return (comment == NULL || colon < comment);
}

/// Returns true if the given line is a valid command, or false otherwise.
static inline bool isCommandLine(const char* line) {
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

/// Returns true if the given line can be ignored, or false otherwise.
static inline bool isIgnoredLine(const char* line) {
    assert(line != NULL);
    size_t first_non_whitespace_char = strspn(line, "\t ");

    if (first_non_whitespace_char == strnlen(line, MAX_LINE_LEN)) return true;
    if (line[first_non_whitespace_char] == '#') return true;

    return false;
}

/**
 * Rule constructor thread. Takes strings and makes them into Rules.
 * 
 * @param args void* wrapping a
 * [Queue* inputQueueOfLines, Queue* outputQueueofRules]
 * 
 * @return void* for thread closure
 */
static void* RuleConstructor(void* args) {
    Queue* in = ((Queue**)args)[0];
    Queue* out = ((Queue**)args)[1];
    assert(in != NULL && out != NULL);

    Rule* rule = NULL;
    struct line* wrapped;
    while ((wrapped = q_dequeue(in)) != NULL) {
        // UNWRAP LINE
        char* line = wrapped->string;
        int linenum = wrapped->linenum;

        // CASE 1: LINE STARTS A NEW TARGET
        if (isTargetLine(line)) {
            // 1. This is the start of a new rule, so get rid of any old one and
            //    make a new one.
            if (rule != NULL) q_enqueue(out, rule);
            rule = newRule();

            // 2. Get the target name, and make a copy to use in the Rule.
            char* target;  // name of target that will be built by this Rule
            char* colon;   // location of ':' delimiter denoting end of target name
            {
                colon = strchr(line, ':');
                size_t targetsize = colon - line;  // as a number of chars

                while (isblank(line[targetsize-1])) targetsize--;

                target = malloc(sizeof(char) * (targetsize + 1));
                if (target == NULL) {
                    exitwitherr(linenum, "Error allocating memory for target name\n", line);
                }

                strncpy(target, line, targetsize);
                target[targetsize] = '\0';
            }

            // 3. Get the dependencies and copy them in an array.
            //    This step uses strtok, so it mutates the input string.
            int numdeps = 0; // number of dependencies found
            char** dependencies; // array of dependencies
            {
                LinkedList* dep_ll = ll_initialize();

                const char dep_delimiters[] = "\t ";
                char* saveptr = NULL;

                // this loop is destructive to the line of input (b/c strtok)
                char* dep = strtok_r(colon + 1, dep_delimiters, &saveptr);
                for (; dep != NULL; dep = strtok_r(NULL, dep_delimiters, &saveptr)) {
                    assert(dep != colon);

                    // use a copy of the string so deps can be individually freed
                    char* cpy = malloc(sizeof(char) * (strlen(dep)));
                    if (cpy == NULL) {
                        exitwitherr(linenum, "Error allocating memory for target name\n", line);
                    }

                    strcpy(cpy, dep);
                    ll_push(dep_ll, cpy);
                }

                numdeps = dep_ll->size;
                dependencies = (char**)ll_to_array(dep_ll);
                ll_destruct(dep_ll);  // free overhead while leaving pointees
            }

            // 4. All needed information found, construct Rule for this target
            rule->target = target;
            rule->linenumber = linenum;
            rule->numdeps = numdeps;
            rule->dependencies = dependencies;
            // rule->commands was initialized by rule constructor, and will be
            // populated by CASE 2
        // CASE 2: LINE IS A COMMAND FOR THE CURRENT TARGET
        } else if (isCommandLine(line)) {
            if (rule == NULL) exitwitherr(linenum, "no target to add command to", line);

            // Copy the command string into the current Rule, cropping spaces
            // off the front and comments off the end
            char* command_string = NULL;
            {
                char* start = line + strspn(line, "\t ");

                // find command end, handling comments and ignoring escaped ones
                char* end = strchrnul(line, '#');
                assert(end > line);
                while (*end == '#' && end[-1] == '\\')
                    end = strchrnul(end+1, '#');

                size_t size = end - start;  // as a number of chars

                if (size <= 0)
                    exitwitherr(linenum, "command must have positive length", line);

                command_string = malloc(sizeof(char) * (size));
                if (command_string == NULL) exitwitherr(linenum, "mem alloc failed", line);

                strncpy(command_string, start, size);
                command_string[size] = '\0';
            }

            Command* command = newCommandFromString(command_string);
            if (command != NULL) ll_push(rule->commands, command);
        } else if (isIgnoredLine(line)) {
            ;  // ignore line
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
 * Grapher thread. Adds Rules to the specified Graph as they are recieved from
 * inputQueue.
 * 
 * @param args void* wrapping a [Queue* inputQueueOfRules, Graph* toAddRulesTo]
 * 
 * @return void* for thread closure
 */
static void* Grapher(void* args) {
    Queue* in = ((Queue**)args)[0];
    Graph* graph = ((Graph**)args)[1];
    assert(in != NULL && graph != NULL);

    Rule* next;

    while ((next = (Rule*)q_dequeue(in)) != NULL) {
        printMakefileRule(next);
        initializeGraphNode(graph, next);
    }

    pthread_exit(NULL);  // return successfully
}

/**
 * Parses a makefile into its dependency graph representation
 * 
 * @param makefile file to parse
 * @return Graph* of makefile Rules that contains all entries from the makefile,
 * or exits gracefully upon error.
 * 
 * @example
 * ```C
 * FILE* makefile = fopen("test/simple_testcase/makefile", "r");
 * Graph* g = ParseMakefile(makefile);
 * ```
 */
Graph* ParseMakefile(FILE* makefile) {
    pthread_t reader_id;
    pthread_t ruleConstructor_id;
    pthread_t grapher_id;

    Graph* graph = initGraph();

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

    return graph;
}