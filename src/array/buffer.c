#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FLAT_INCLUDES
#include "range.h"
#include "buffer.h"

int _buffer_resize (buffer_void * expand_buffer, size_t type_size, size_t new_count)
{
    size_t new_size = type_size * new_count;
    void * new_region = realloc (expand_buffer->begin, new_size);
    if (!new_region)
    {
	perror ("realloc");
	fflush (stderr);
	return -1;
    }
    size_t range_size = range_count (*expand_buffer);
    *expand_buffer = (buffer_void) { .begin = new_region,
				     .end = new_region + range_size,
				     .max = new_region + new_size };
    return 0;
}

void _buffer_downshift (buffer_void * buffer, size_t element_size, size_t count)
{
    assert (range_count(*buffer) % element_size == 0);

    size_t buffer_size = range_count(*buffer);

    size_t size = element_size * count;

    if (buffer_size <= size)
    {
	buffer_rewrite (*buffer);
	return;
    }

    size_t shift_size = buffer_size - size;

    assert (shift_size < buffer_size);
    assert (shift_size);

    memmove (buffer->begin, (char*)buffer->begin + size, shift_size);

    buffer->end = (char*)buffer->end - size;

    assert ((size_t)range_count (*buffer) == shift_size);
}
