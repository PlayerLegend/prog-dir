#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FLAT_INCLUDES
#include "range.h"
#include "buffer.h"

int _buffer_realloc (buffer_void * expand_buffer, size_t type_size, size_t new_count)
{
    size_t new_size = type_size * new_count;

    size_t have_size = (char*)expand_buffer->max - (char*)expand_buffer->begin;

    if (have_size >= new_size)
    {
	return 0;
    }

    new_size = 4 * new_size + 10 * type_size;
    
    assert (new_size > 0);
    
    void * new_region = realloc (expand_buffer->begin, new_size);

    if (!new_region)
    {
	perror ("realloc");
	//fprintf(stderr, "tried to alloc %zu * %zu = %zu", type_size, new_count, type_size * new_count);
	fflush (stderr);
	return -1;
    }

    size_t range_size = range_count (*expand_buffer);

    *expand_buffer = (buffer_void)
	{ .begin = new_region,
	  .end = (char*)new_region + range_size,
	  .max = (char*)new_region + new_size };
    
    return 0;
}

void _buffer_downshift (buffer_void * buffer, size_t element_size, size_t count)
{
    assert (range_count(*buffer) % element_size == 0);

    if (!count)
    {
	return;
    }
    
    size_t buffer_size = range_count(*buffer);

    size_t size = element_size * count;

    if (buffer_size <= size)
    {
	buffer_rewrite (*buffer);
	return;
    }

    assert (buffer_size > size);
    
    size_t shift_size = buffer_size - size;

    assert (shift_size < buffer_size);
    assert (shift_size);

    memmove (buffer->begin, (char*)buffer->begin + size, shift_size);

    buffer->end = (char*)buffer->end - size;

    assert ((size_t)range_count (*buffer) == shift_size);
}

void buffer_strncpy (buffer_char * buffer, const char * begin, size_t size)
{
    buffer_resize(*buffer, size + 1);
    memcpy (buffer->begin, begin, size);
    buffer->end--;
    *buffer->end = '\0';
}

void buffer_strcpy (buffer_char * to, const char * input)
{
    size_t size = strlen(input) + 1;
    buffer_resize (*to, size);
    memcpy (to->begin, input, size - 1);
    to->end--;
    *to->end = '\0';
}
