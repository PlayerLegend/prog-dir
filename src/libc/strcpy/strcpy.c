#include <sys/types.h>
#define FLAT_INCLUDES
#include "../string.h"

char *strcpy(char *dest, const char *src)
{
    size_t len = strlen (src);

    memcpy (dest, src, len);
    dest[len] = '\0';

    return dest;
}
