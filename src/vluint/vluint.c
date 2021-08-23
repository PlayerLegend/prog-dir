#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "vluint.h"
#include "../log/log.h"

#define VLUINT_CHAR_MAX 128

keyargs_define(vluint_read)
{
    vluint_result result = 0;
    
    const unsigned char * i;
    unsigned char c;

    vluint_result power = 1;

    for_range (i, *args.input)
    {
	c = *(char*)i;

	result += power * (c & ~VLUINT_CHAR_MAX);
        
	if ( !(c & VLUINT_CHAR_MAX) )
	{
	    if (args.vluint)
	    {
		args.vluint->begin = args.input->begin;
		args.vluint->end = i + 1;
	    }

	    if (args.rest)
	    {
		args.rest->begin = i + 1;
		args.rest->end = args.input->end;
	    }

	    if (args.result)
	    {
		*args.result = result;
	    }

	    return true;
	}
	
	power *= VLUINT_CHAR_MAX;
    }

    //log_error ("vluint is truncated");

    return false;
}

void vluint_write(buffer_unsigned_char * output, vluint_result n)
{
    //buffer_rewrite (*output);

    while (n >= VLUINT_CHAR_MAX)
    {
	*buffer_push(*output) = (n % VLUINT_CHAR_MAX) | VLUINT_CHAR_MAX;
	n = n >> 7;

	assert (output->end[-1] & VLUINT_CHAR_MAX);
    }

    *buffer_push(*output) = n;
}
