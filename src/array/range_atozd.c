#include <stddef.h>
#include <stdbool.h>
#include <ctype.h>
#define FLAT_INCLUDES
#include "range.h"

size_t range_atozd (size_t * restrict output, const range_const_char * restrict input)
{
    const char * i = input->begin;

    *output = 0;
    
    while (i < input->end && isdigit (*i))
    {
	*output = 10 * (*output) + (*i - '0');
	i++;
    }

    return range_index (i, *input);
}
