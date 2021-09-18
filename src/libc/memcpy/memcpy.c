#include <sys/types.h>
#include <stdint.h>
#define FLAT_INCLUDES
#include "../string.h"

void * memcpy(void * const dest_orig, const void * const src_orig, const size_t n_orig)
{
    size_t n = n_orig;
    
    typedef uint_fast32_t fast_int;

    union {
	uint8_t * i8;
	fast_int * fast;
	uintptr_t ival;
    }
	dest = { .i8 = dest_orig };

    union {
	const uint8_t * i8;
	const fast_int * fast;
	const uintptr_t ival;
    }
	src = { .i8 = src_orig };

    while (src.ival % sizeof(fast_int) != 0 && n != 0)
    {
	*dest.i8 = *src.i8;
	dest.i8++;
	src.i8++;
	n--;
    }
    
    //if (intptr_destp % sizeof(fast_int) == 0)
    {
	while (n > 4 * sizeof(fast_int))
	{
	    dest.fast[0] = src.fast[0];
	    dest.fast[1] = src.fast[1];
	    dest.fast[2] = src.fast[2];
	    dest.fast[3] = src.fast[3];

	    dest.fast += 4;
	    src.fast += 4;
	    n -= 4 * sizeof(fast_int);
	}

	while (n > sizeof(fast_int))
	{
	    *dest.fast = *src.fast;

	    dest.fast++;
	    src.fast++;
	    n -= sizeof(fast_int);
	}

	while (n > 0)
	{
	    *dest.i8 = *src.i8;
	    dest.i8++;
	    src.i8++;
	    n--;
	}

	return dest_orig;
    }
    
}
