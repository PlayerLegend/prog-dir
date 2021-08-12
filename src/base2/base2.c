#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "base2.h"

#include "../log/log.h"

void base2_encode(buffer_char * output, const range_const_unsigned_char * input)
{
    const unsigned char * i;
    
    int bit;
    unsigned char c;

    for_range (i, *input)
    {
	c = *i;

	for (bit = 1; bit < 256; bit <<= 1)
	{
	    *buffer_push (*output) = c & bit ? '1' : '0';
	}
    }
}

bool base2_decode (buffer_unsigned_char * output, const range_const_char * input)
{
    const char * i;
    char c;
    char byte;
    char bit;
    unsigned char byte_index = 0;

    if (range_count (*input) % 8 != 0)
    {
	log_error ("base2 string's length is not a multiple of 8");
	return false;
    }
    
    for_range (i, *input)
    {
	c = *i;

	byte_index = (i - input->begin) % 8;
	
	if (byte_index == 0)
	{
	    byte = 0;
	    bit = 1;
	}

	if (c == '1')
	{
	    byte |= bit;
	}
	else if (c != '0')
	{
	    log_error ("Invalid character in base2 string: %c", c);
	    return false;
	}

	if (byte_index == 7)
	{
	    *buffer_push (*output) = byte;
	}
	else
	{
	    bit <<= 1;
	}
    }

    assert (byte_index == 7);
    
    return true;
}
