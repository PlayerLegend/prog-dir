#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <unistd.h>

//#include "range.h"
#include "stack.h"
#include "array.h"
#include "nc-db/buffer.h"

ssize_t buffer_grow(char_array * array, int fd)
{
    ssize_t rate = 1 << 16;

    ssize_t got = 0;

    ssize_t need;
    
    ssize_t total_got = 0;

    do {
	total_got += got;
	array->end += got;
	need = count_range(*array) + rate + 1;
	array_alloc(array,need);
    }
    while( -1 != (got = read(fd,array->end,rate)) );

    *array->end = '\0';

    return total_got;
}

char * buffer_extract(char_array * array, char * begin, char delim)
{
    char * end;
    if(!begin)
    {
	begin = array->begin;
	end = strchr(begin,delim);
	if(end)
	{
	    *end = '\0';
	    return begin;
	}
	else
	{
	    return NULL;
	}
    }
    else
    {
	while(begin < array->end && *begin)
	    begin++;

	begin++;

	if(begin >= array->end)
	    return NULL;

	end = strchr(begin,delim);
	if(end)
	{
	    *end = '\0';
	    return begin;
	}
	else
	{
	    size_t shift = (begin - array->begin) + strlen(begin) + 1;
	    size_t size = count_range(*array) - shift;
	    memmove(array->begin,array->begin + shift,size);
	    array->end = array->begin + size;
	    return NULL;
	}
    }
}
