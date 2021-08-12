#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef size_t vluint_result;
typedef char vluint[];

keyargs_declare(bool, vluint_read, const range_const_unsigned_char * input; range_const_unsigned_char *vluint, *rest; vluint_result * result;);
#define vluint_read(...) keyargs_call(vluint_read, __VA_ARGS__)
void vluint_write(buffer_unsigned_char * output, vluint_result n);
