#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "arch.h"
#include "constants.h"

static bool mem_compare (void * a, void * b, size_t size)
{
    while (size--)
    {
	if ( *(char*) a != *(char*) b )
	{
	    return false;
	}

	a = ((char*)a) + 1;
	b = ((char*)b) + 1;
    }

    return true;
}

static bool find_constant (vm_address * result, buffer_char * constants, void * needle, size_t needle_size)
{
    if (!constants->begin)
    {
	return false;
    }
    
    range_char scan = { constants->begin, constants->end - needle_size };

    char * match;
    for_range (match, scan)
    {
	if (mem_compare (match, needle, needle_size))
	{
	    *result = match - constants->begin;
	    return true;
	}	
    }
    
    return false;
}

vm_address record_constant (buffer_char * constants, void * constant_value, size_t constant_size)
{
    vm_address retval;

    if (find_constant (&retval, constants, constant_value, constant_size))
    {
	return retval;
    }
    else
    {
	retval = range_count (*constants);
	size_t new_size = retval + constant_size;
	buffer_resize(*constants, new_size);
	memcpy (constants->begin + retval, constant_value, constant_size);
	return retval;
    }
}
