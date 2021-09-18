#include <sys/types.h>
#include <stdint.h>
#define FLAT_INCLUDES
#include "string.h"

typedef uint_fast32_t fast_int;

#define char_dest ((char*) dest)
#define char_src ((const char*) src)

#define int_dest ((fast_int*) dest)
#define int_src ((fast_int*) src)

#define intptr_destp ((uintptr_t) dest)

void * memcpy(void *dest_orig, const void *src_orig, size_t n)
{
        
    void * dest_orig = dest;
    
    /*while (intptr_destp % sizeof(fast_int) && n != 0)
    {
	*char_dest = *char_src;
	dest = char_dest + 1;
	src = char_src + 1;
	n--;
	}*/

    //if (intptr_destp % sizeof(fast_int) == 0)
    {
	while (n > 4 * sizeof(fast_int))
	{
	    int_dest[0] = int_src[0];
	    int_dest[1] = int_src[1];
	    int_dest[2] = int_src[2];
	    int_dest[3] = int_src[3];

	    dest = int_dest + 4;
	    src = int_src + 4;
	    
	    n -= 4 * sizeof(fast_int);
	}

	/*if (n & 2 * sizeof(fast_int))
	{
	    int_dest[0] = int_src[0];
	    int_dest[1] = int_src[1];

	    dest = int_dest + 1;
	    src = int_src + 1;
	}

	if (n & sizeof(fast_int))
	{
	    *int_dest = *int_src;
	    dest = int_dest + 1;
	    src = int_src + 1;
	}

	while (n % sizeof(fast_int))
	{
	    *char_dest = *char_src;
	    dest = char_dest + 1;
	    src = char_src + 1;
	    n++;
	    }*/

	while (n > sizeof(fast_int))
	{
	    int_dest[0] = int_src[0];

	    dest = int_dest + 1;
	    src = int_src + 1;

	    n -= sizeof(fast_int);
	}

	while (n > 0)
	{
	    *char_dest = *char_src;

	    dest = char_dest + 1;
	    src = char_src + 1;
	    
	    n--;
	}

	return dest_orig;
    }
    
}
