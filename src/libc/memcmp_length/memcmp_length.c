#include <stdint.h>
#include <stddef.h>
#define FLAT_INCLUDES
#include "string-extensions.h"

size_t memcmp_length (const void * a_orig, const void * b_orig, size_t size_orig)
{
    typedef uint_fast32_t fast_int;
    union {
	const fast_int * fast;
	const unsigned char * c;
	uintptr_t ival;
    }
	a = { .c = a_orig },
	b = { .c = b_orig };

    size_t size = size_orig;

    while (a.ival % sizeof(fast_int) && size)
    {
	if (*a.c != *b.c)
	{
	    return size_orig - size;
	}

	a.c++;
	b.c++;
	size--;
    }

    while (size >= sizeof(fast_int))
    {
	if (*a.fast != *b.fast)
	{
	    break;
	}

	a.fast++;
	b.fast++;
	size -= sizeof(fast_int);
    }

    while (*a.c != *b.c && size)
    {
	if (*a.c != *b.c)
	{
	    break;
	}

	a.c++;
	b.c++;
	size--;
    }

    return size_orig - size;
}
