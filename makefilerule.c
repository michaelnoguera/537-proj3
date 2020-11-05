#define _GNU_SOURCE

#include "makefilerule.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>

/// Consructs a new Rule struct with default values
Rule* newRule() {
    Rule* rule = malloc(sizeof(Rule));
    if (rule == NULL) {
        perror("Failed to allocate memory for new Rule.");
        exit(EXIT_FAILURE);
    }

    rule->target = NULL;
    rule->numdeps = -1;
    rule->dependencies = NULL;
    rule->linenumber = -1;
    rule->commands = ll_initialize();

    return rule;
}

/// Constructs a new Command struct with default values
Command* newCommand() {
    Command* command = malloc(sizeof(Command));
    if (command == NULL) {
        perror("Failed to allocate memory for new Command.");
        exit(EXIT_FAILURE);
    }

    command->argv = NULL;
    command->inputfile = NULL;
    command->outputfile = NULL;

    return command;
}

// static const size_t MAX_LINE_LEN = 4096;

/*static void printSubstring(char* string, char* start, char* end, int color) {
    for (size_t i = 0; i < strlen(string); i++) {
        if (start == string + i) printf("\x1B[30;%im", color);
        printf("%c", string[i]);
        if (end == string + i) printf("\x1B[m");
    }
    printf("\x1B[0m\n");
}*/

static const char delimiters[] = " <>\0";

/**
 * Advances a char* to the next token for input
 * @param[in/out] mut char pointer to advance
 * @param[out] IOreached 1 if '<' or '>' reached
 * @param[out] endreached 1 if '\0' at end of string reached
 */
static char* advanceToStartOfToken(char** mut, bool* IOreached,
                                   bool* endreached) {
    assert(mut != NULL && *mut != NULL);
    while (**mut != '\0'
           && (**mut == ' ' || **mut == '\t' || **mut == '<' || **mut == '>')) {
        if (**mut == '<' || **mut == '>') {
            *IOreached = true;
            return *mut;
        }
        *mut = *mut + 1; // autoscales to char
    }
    if (**mut == '\0') { *endreached = true; }
    *IOreached = false;
    return *mut;
}

/**
 * Given a string containing a makefile command, tokenize it and construct a new
 * Command struct with the appropriate fields
 *
 * @param[in] string
 *
 * @return command struct
 */
Command* newCommandFromString(char* string) {
    if (string == NULL) {
        perror("exec: <Cannot construct command from NULL string>");
        exit(EXIT_FAILURE);
    }
    if (strspn(string, " \t") == strlen(string)) {
        perror(
          "exec: <Empty string not a valid command, probably an issue in "
          "MakefileParser>");
        exit(EXIT_FAILURE);
    }

    Command* command = newCommand();
    LinkedList* argv_ll = ll_initialize();

    command->inputfile = NULL;
    command->outputfile = NULL;

    // Find some key points in the string (or assert their absence) in an
    // initial pass
    char* leftarrow = strchrnul(string, '<');
    char* rightarrow = strchrnul(string, '>');
    char* nullterminator = strrchr(string, '\0');
    assert(nullterminator != NULL); // string should be null terminated

    // 1. extract arguments which follow the command name
    char* start = string; // start of current section being parsed
    char* end = string;   // end of current section being parsed
    bool IOreached = false;
    bool endreached = false;
    {
        while (*end != '\0' && endreached == false) {
            // A) find argument bounds
            start = end; // find start of arg
            advanceToStartOfToken(&start, &IOreached, &endreached);
            if (IOreached) break;
            if (endreached) break; // *start == '\0', so stop
            assert(start != NULL && *start != '\0');

            end = start + strcspn(start, delimiters) - 1;

            // B) calculate argument string length
            size_t size = end - start + 1; // as a number of chars
            assert(size > 0);

            // C) copy argument into a new variable
            char* arg = malloc(sizeof(char) * (size + 1));
            if (arg == NULL) {
                perror(
                  "exec: <Memory allocation failed while interpreting "
                  "command>");
                exit(EXIT_FAILURE);
            }

            strncpy(arg, start, size);
            arg[size] = '\0';

            // D) add the variable to arguments list
            ll_push(argv_ll, arg);

            // E) advance pointer so that next iteration doesn't hit same token
            end++;
        }
    }

    // Convert linkedlist into array as per argv needs
    command->argv = (char**)ll_to_array(argv_ll);
    ll_destruct(argv_ll); // free overhead without hurting string*s used by argv

    if (IOreached) {
        // 2. I/O redirection: CMD < INPUT
        if (leftarrow != nullterminator) {
            // A) find token bounds
            start = leftarrow + 1;

            // in case input is "<>" or "> <" etc.
            bool noActualFileSpecified = false;

            advanceToStartOfToken(&start, &noActualFileSpecified, &endreached);
            if (noActualFileSpecified || endreached) {
                perror("exec: <Invalid I/O redirection>");
                exit(EXIT_FAILURE);
            }

            end = start + strcspn(start, delimiters) - 1;

            // B) calculate string length
            size_t size = end - start + 1; // as a number of chars
            assert(size > 0);

            // C) copy string into place
            command->inputfile = malloc(sizeof(char) * (size + 1));
            if (command->inputfile == NULL) {
                perror(
                  "exec: <Memory allocation failed while interpreting "
                  "command>");
                exit(EXIT_FAILURE);
            }

            strncpy(command->inputfile, start, size);
            command->inputfile[size] = '\0';
        }

        // 3. I/O redirection: CMD > OUTPUT
        if (rightarrow != nullterminator) {
            // A) find token bounds
            start = rightarrow + 1;

            // in case input is "<>" or "> <" etc.
            bool noActualFileSpecified = false;

            advanceToStartOfToken(&start, &noActualFileSpecified, &endreached);
            if (noActualFileSpecified || endreached) {
                perror("exec: <Invalid I/O redirection>");
                exit(EXIT_FAILURE);
            }

            end = start + strcspn(start, delimiters) - 1;

            // B) calculate string length
            size_t size = end - start + 1; // as a number of chars
            assert(size > 0);

            // C) copy string into place
            command->outputfile = malloc(sizeof(char) * (size + 1));
            if (command->outputfile == NULL) {
                perror(
                  "exec: <Memory allocation failed while interpreting "
                  "command>");
                exit(EXIT_FAILURE);
            }

            strncpy(command->outputfile, start, size);
            command->outputfile[size] = '\0';
        }
    }

    return command;
}

inline void printMakefileRule(Rule* r) {
    if (r == NULL) return;
    printf("\n\x1B[1;4m%s\x1B[1;24m (%p)\x1B[0m\n", r->target, (void*)r);
    printf("\x1B[96mfrom line\x1B[0m = %i\n", r->linenumber);
    printf("\x1B[96mnumdeps\x1B[0m = %i\n", r->numdeps);
    if (r->numdeps > 0) {
        printf("\x1B[96mdependencies: \x1B[4mchar* array\x1B[0m");
        const int NODES_ON_LINE = 5;
        for (int i = 0; i < r->numdeps; i++) {
            if (i == 0 || i % NODES_ON_LINE == 0) printf("\n\t");
            printf("\x1B[33m%i.\x1B[0m\"\x1B[1m%s\x1B[0m\"\x1B[2m, \x1B[0m", i,
                   r->dependencies[i]);
        }
        printf("\n");
    }

    if (r->commands != NULL && r->commands->size != 0) {
        printf("\x1B[96mcommands: ");
        ll_print_as_strings(r->commands);
    }
}