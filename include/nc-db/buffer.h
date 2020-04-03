#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "range.h"
#include "stack.h"
#include "array.h"

#endif

ssize_t buffer_grow(char_array * array, int fd);
char * buffer_extract(char_array * array, char * begin, char delim);
