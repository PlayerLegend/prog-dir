#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "range.h"

size_t range_strstr (const range_const_char * haystack, const range_const_char * needle)
{
    size_t needle_size = range_count (*needle);

    if (!needle_size)
    {
	return range_count (*haystack);
    }
    
    range_const_char scan = { .begin = haystack->begin, .end = haystack->end - needle_size + 1 };

    const char * i_scan;

    size_t i;

    for_range (i_scan, scan)
    {
	i = 0;

	while (i < needle_size)
	{
	    if (i_scan[i] != needle->begin[i])
	    {
		break;
	    }

	    i++;
	}

	if (i == needle_size)
	{
	    break;
	}
    }

    if (i_scan == scan.end)
    {
	return range_count (*haystack);
    }
    else
    {
	return range_index (i_scan, scan);
    }
}

size_t range_strchr (const range_const_char * haystack, char needle)
{
    const char * i;

    for_range (i, *haystack)
    {
	if (*i == needle)
	{
	    break;
	}
    }

    return range_index(i, *haystack);
}

bool range_streq (const range_const_char * a, const range_const_char * b)
{
    size_t size = range_count (*a);

    if (size != (size_t) range_count (*b))
    {
	return false;
    }

    for (size_t i = 0; i < size; i++)
    {
	if (a->begin[i] != b->begin[i])
	{
	    return false;
	}
    }

    return true;
}
