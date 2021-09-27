#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

/** @file base2/base2.h

    This file provides functions for encoding and decoding base2 strings, where 1 is represented by ascii '1' and 0 by ascii '0'
*/

void base2_encode(buffer_char * output, const range_const_unsigned_char * input); ///< Encodes real binary input into ascii binary output
bool base2_decode (buffer_unsigned_char * output, const range_const_char * input); ///< Encodes ascii binary input into real binary output
