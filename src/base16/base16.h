#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

/** @file base16/base16.h

    This file provides functions for encoding and decoding hexadecimal strings
*/

void base16_encode (buffer_char * output, const range_const_unsigned_char * input, bool little_endian); ///< Encodes binary input into hexadecimal output. The endianness option changes the order of the output hex character pairs. This function outputs lowercase hexadecimal characters.
bool base16_decode (buffer_unsigned_char * output, const range_const_char * input, bool little_endian); ///< This function decodes hexadecimal numbers and works with both upper and lower case input
