#include <sys/types.h>
#define FLAT_INCLUDES
#include "string.h"

size_t strlen(const char *s)
{
    const char * end = s;

    while (*end)
    {
	end++;
    }

    return end - s;
}
