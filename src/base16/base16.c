#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "base16.h"

#include "../log/log.h"

void base16_encode(buffer_char * output, const range_const_unsigned_char * input, bool little_endian)
{
    const unsigned char * c;
    char result[2];
    unsigned char i;
    
    for_range (c, *input)
    {
	result[0] = *c % 16;
	result[1] = *c / 16;

	for (i = 0; i <= 1; i++)
	{
	    if (result[i] <= 9)
	    {
		result[i] += '0';
	    }
	    else
	    {
		result[i] += 'a' - 10;
	    }
	}
	
	*buffer_push(*output) = result[!little_endian];
	*buffer_push(*output) = result[little_endian];
    }
}

bool base16_decode (buffer_unsigned_char * output, const range_const_char * input, bool little_endian)
{
    const char * c;
    int i;

    unsigned char inchar_result[2];

    for (c = input->begin; c < input->end; c += 2)
    {
	for (i = 0; i <= 1; i++)
	{
	    if (c + i >= input->end)
	    {
		inchar_result[i] = 0;
	    }
	    else if ('0' <= c[i] && c[i] <= '9')
	    {
		inchar_result[i] = (c[i] - '0');
	    }
	    else if ('a' <= tolower(c[i]) && tolower(c[i]) <= 'f')
	    {
		inchar_result[i] = 10 + (tolower(c[i]) - 'a');
	    }
	    else
	    {
		log_fatal ("Invalid char in base16 string");
	    }

	    assert (inchar_result[i] >= 0);
	    assert (inchar_result[i] <= 16);
	}

	*buffer_push(*output) = inchar_result[!little_endian] + inchar_result[little_endian] * 16;
    }
    
    return true;

fail:
    return false;
}
