#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "path.h"
#include "../log/log.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"

/*
static void buffer_strcpy (buffer_char * to, const char * input)
{
    size_t size = strlen(input) + 1;
    buffer_resize (*to, size);
    memcpy (to->begin, input, size - 1);
    to->end--;
    *to->end = '\0';
    }*/

inline static bool _mkdir (const char * path)
{
    if (-1 == mkdir (path, 0755) && errno != EEXIST)
    {
	perror (path);
	return false;
    }
    else
    {
	return true;
    }
}

bool _path_mkdir (char * path, bool make_target)
{
    char * i = path;

    if (*i == PATH_SEPARATOR)
    {
	i++;
    }

    while (*i)
    {
	if (*i == PATH_SEPARATOR)
	{
	    *i = '\0';
	    if (!_mkdir (path))
	    {
		//perror (path);
		*i = PATH_SEPARATOR;
		log_fatal ("Could not create a directory for %s", path);
	    }
	    *i = PATH_SEPARATOR;
	}
	
	i++;
    }

    if (make_target && !_mkdir (path))
    {
	//perror (path);
	log_fatal ("Could not create directory %s", path);
    }

    return true;

fail:
    return false;
}

void path_cat (buffer_char * buffer, const char * a, const char * b)
{
    size_t a_len = strlen(a);

    while (a_len && a[a_len - 1] == PATH_SEPARATOR)
    {
	a_len--;
    }

    if (!a_len)
    {
	buffer_printf (buffer, "%s", b);
	return;
    }

    while (*b == PATH_SEPARATOR)
    {
	b++;
    }

    buffer_printf (buffer, "%.*s/%s", a_len, a, b);
}
