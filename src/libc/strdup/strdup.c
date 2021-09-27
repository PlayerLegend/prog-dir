#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../string-extensions.h"
#include "../string.h"

char * strdup (const char * input)
{
    size_t len = strlen (input);
    char * mem = malloc (len + 1);

    if (!mem)
    {
	perror ("malloc");
	return NULL;
    }
    
    return strcpy (mem, input);
}
