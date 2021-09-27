#include <sys/types.h>
#define FLAT_INCLUDES
#include "../string.h"

char *strncpy(char *dest, const char *src, size_t n)
{
    size_t len = strlen (src);

    memcpy (dest, src, len);

    for (size_t i = len; i < n; i++)
    {
	dest[i] = '\0';
    }
    
    return dest;
}
