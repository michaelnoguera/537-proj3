#define _GNU_SOURCE

#include "makefilerule.h"

#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

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

//static const size_t MAX_LINE_LEN = 4096;

static void printSubstring(char* string, char* start, char* end, int color) {
    for (size_t i = 0; i < strlen(string); i++) {
        if (start == string+i) printf("\x1B[30;%im", color);
        printf("%c", string[i]);
        if (end == string+i) printf("\x1B[m");
    }
    printf("\x1B[0m\n");
}

static inline int max(int a, int b) {
    if (a >= b) return a;
    else if (b > a) return b;
}

static const char delimiters[] = " <>\0";

/**
 * Advances a char* to the next token for input
 * @param[in/out] mut char pointer to advance
 * @param[out] 1 if '<' or '>' reached
 */
static char* advanceToStartOfToken(char* mut, bool* IOreached) {
    assert(mut != NULL && IOreached == false);
    while (*mut == ' ' || *mut == '\0' || *mut == '\t' || *mut == '<' || *mut == '>') {
        if (*mut == '<' || *mut == '>') {
            IOreached = true;
            return;
        }
        return mut+1; // autoscales to char
    }
}

Command* newCommandFromString(char* string) {
    if (string == NULL) {
        perror("exec: <Cannot construct command from NULL string>");
        exit(EXIT_FAILURE);
    }
    if (strspn(string, " \t") == strlen(string)) {
        perror("exec: <Empty string not a valid command, probably an issue in MakefileParser>");
        exit(EXIT_FAILURE);
    }

    Command* command = newCommand();
    char* start = string; // start of current section being parsed
    char* end = string; // end of current section being parsed
    
    bool IOreached = false;
    char* leftarrow = strchrnul(string, '<');
    char* rightarrow = strchrnul(string, '>');

    LinkedList* argv_ll = ll_initialize();
    // 1. extract arguments which follow the command name
    {
        while (*end != '\0') {
            // A) Find start and end of current token
            start = advanceToStartOfToken(&start, &IOreached); // find start of arg
            if (IOreached) break;

            end = start + strcspn(start, delimiters)-1;  // auto-scaled to char size

            size_t size = end - start + 1;  // as a number of chars
            printSubstring(string, start, end, 102);
            assert(size > 0);

            printSubstring(string, start, end, 102);

            // B) Copy token into place
            char* arg = malloc(sizeof(char) * (size+1));
            if (arg == NULL) {
                perror("exec: <Memory allocation failed while interpreting command>");
                exit(EXIT_FAILURE);
            }

            strncpy(arg, start, size);
            arg[size] = '\0';

            end++;

            ll_push(argv_ll, arg);
            ll_print_as_strings(argv_ll);
        }
    }
    ll_print_as_strings(argv_ll);
    command->argv = (char**)ll_to_array(argv_ll);

    command->outputfile = NULL;
    {
        char* start = leftarrow;
        char* end = max(rightarrow, string+strlen(string));
        // if there is an output redirect
        if (start < end) {
            printSubstring(string, start, end, 107);
        }
    }

    command->inputfile = NULL;
    {
        char* start = rightarrow;
        char* end = max(leftarrow, string+strlen(string));
        // if there is an input redirect
        if (start < end) {
            printSubstring(string, start, end, 107);

            /*char* arg = malloc(sizeof(char) * (size+1));
            if (arg == NULL) {
                perror("exec: <Memory allocation failed while interpreting command>");
                exit(EXIT_FAILURE);
            }

            strncpy(arg, start, size);
            arg[size] = '\0';*/

        }
    }

    /*
    // 2. find I/O redirect filenames
    //if (strstr())
    while (*end != '\0') {
        start = end;
        while (*start == ' ' || *start == '\0' || *start == '\t'){
            //printSubstring(string, start, start, 101);
            start++;
        }
        printf("\x1B[31m start: %c \x1B[0m\n", *start);
        end = start + strcspn(start, delimiters);  // auto-scaled to char size
        assert(end > start);

        //printSubstring(string, start, end, 103);
        while (*end == ' ' || *end == '\0' || *end == '\t'){
            //printSubstring(string, end, end, 101);
            end--;
        }

        size_t size = end - start + 1;  // as a number of chars
        assert(size > 0);

        //printSubstring(string, start, end, 102);

        char* arg = malloc(sizeof(char) * (size+1));
        if (arg == NULL) {
            perror("Memory allocation failed while interpreting command");
            exit(EXIT_FAILURE);
        }

        strncpy(arg, start, size);
        arg[size] = '\0';

        command->inputfile = arg;

        end++;
    }
    // manually*/

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
            printf("\x1B[33m%i.\x1B[0m\"\x1B[1m%s\x1B[0m\"\x1B[2m, \x1B[0m", i, r->dependencies[i]);
        }
        printf("\n");
    }

    if (r->commands != NULL && r->commands->size != 0) {
        printf("\x1B[96mcommands: ");
        ll_print_as_strings(r->commands);
    }
}