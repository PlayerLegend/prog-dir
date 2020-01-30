#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#include "stack.h"
#include "array.h"
#include <stdbool.h>
#endif

typedef struct {
    char * whitespace;
    char * quote;
    char escape;
}
    delimit_config;

typedef array(char*) delimited_list;

typedef struct {
    char * separator_list;
    int separator_count;
    bool fail_empty_subject;
    bool fail_empty_predicate;
}
    clause_config;

typedef struct {
    char * subject;
    char * predicate;
}
    clause;

int delimit_list(delimited_list * out, const delimit_config * config, const char * input);
int delimit_clause(clause * out, const clause_config * config, const char * input);
void delimit_terminate(char * text, char end);

void clear_clause(void * target);
void clear_delimited_list(void * target);
