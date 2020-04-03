#ifndef FLAT_INCLUDES
#include <stdio.h>
#define FLAT_INCLUDES
#include "range.h"
#include "stack.h"
#include "array.h"
#endif

void clean_trailing(char * string, char targets);

typedef array(char*) delimited_string;

void delimit(delimited_string * out, char * input, char delim);

void print_delimited(delimited_string * print);
