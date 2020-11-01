#include "makefilerule.h"

Rule* newRule() {
    Rule* rule = malloc(sizeof(Rule));

    rule->target = NULL;
    rule->numdeps = -1;
    rule->dependencies = NULL;
    rule->linenumber = -1;
    rule->commands = ll_initialize();

    return rule;
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

    if (r->commands != NULL && r->commands->size != 0){
        printf("\x1B[96mcommands: ");
        ll_print_as_strings(r->commands);
    }
}