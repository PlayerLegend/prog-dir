#ifndef FLAT_INCLUDES
#include <stdio.h>
#include "array.h"
#endif

typedef struct {
    array(char*);
    
    struct {
	char * text;
	size_t len;
    }
	line;
}
    line_args;

int read_line_args(line_args * args, FILE * file);
