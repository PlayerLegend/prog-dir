#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

void base16_encode(buffer_char * output, const range_const_unsigned_char * input);
bool base16_decode (buffer_unsigned_char * output, const range_const_char * input);
