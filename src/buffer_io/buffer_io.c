#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "buffer_io.h"
#include "../log/log.h"

inline static long int read_alloc_size (buffer_char * buffer, int fd)
{
    assert (buffer->end < buffer->max - 1);

    size_t fill_size = buffer->max - 1 - buffer->end;
    size_t have_size = range_count (*buffer);

    assert (fill_size + have_size == (size_t)(buffer->max - 1 - buffer->begin));

    char * fill_point = buffer->begin;
    buffer->end = buffer->begin + have_size + fill_size;
    return read (fd, fill_point, fill_size);
}

keyargs_define(buffer_read)
{
    assert (args.buffer);
    
    size_t have_size = range_count(*args.buffer);
    char * fill_point = 0;
    size_t fill_size = 0;

    size_t max_buffer_size = args.max_buffer_size ? args.max_buffer_size : -1;

    if (have_size >= max_buffer_size)
    {
	*buffer_push (*args.buffer) = '\0';
	args.buffer->end--;
	return 0;
    }
    else if (args.buffer->max && args.buffer->end < args.buffer->max - 1)
    {
	fill_point = args.buffer->end;
	fill_size = (args.buffer->max - 1) - fill_point;
    }
    else
    {
	fill_size = have_size ? have_size : (args.initial_alloc_size ? args.initial_alloc_size : 1000);
	size_t new_size = have_size + fill_size + 1;

	if (0 > buffer_resize (*args.buffer, new_size))
	{
	    return -1;
	}
	
	fill_point = args.buffer->begin + have_size;
    }

    if (have_size + fill_size > max_buffer_size)
    {
	fill_size = max_buffer_size - have_size;
    }
    
    ssize_t read_result = read (args.fd, fill_point, fill_size);

    if (read_result < 0)
    {
	perror ("buffer_read");
	return read_result;
    }

    args.buffer->end = fill_point + read_result;
    assert (args.buffer->end >= args.buffer->begin);
    assert (args.buffer->max);
    assert (fill_point + fill_size <= args.buffer->max - 1);
    assert (args.buffer->end <= args.buffer->max - 1);
    *args.buffer->end = '\0';

    return read_result;
}

 /*
keyargs_define(buffer_read)
{
    assert (args.buffer);
    
    size_t have_size = range_count(*args.buffer);

    if (args.max_buffer_size && have_size >= args.max_buffer_size)
    {
	int plusone = range_count (*args.buffer) + 1;
	buffer_realloc (*args.buffer, plusone);
	*args.buffer->end = '\0';
	return 0;
    }

    //size_t alloc_size = args.buffer->max - args.buffer->begin;
    
    size_t min_add_size = args.initial_alloc_size ? args.initial_alloc_size : 1000;
    size_t add_size = have_size;
    
    //log_debug ("add size: %zu vs %zu", min_add_size, min_add_size);

    if (add_size < min_add_size)
    {
	add_size = args.initial_alloc_size ? args.initial_alloc_size : 1000;
	//add_size = min_add_size;
    }

    size_t new_size = have_size + add_size + 1;

    if (args.max_buffer_size && new_size > args.max_buffer_size)
    {
	new_size = args.max_buffer_size + 1;
	add_size = (new_size - 1) - have_size;
	assert (new_size > have_size);
    }

    buffer_resize (*args.buffer, new_size);

    char * new = args.buffer->begin + have_size;

    long int retval = read (args.fd, new, add_size);

    if (retval >= 0)
    {
	args.buffer->end = args.buffer->begin + have_size + retval;
    }
    else
    {
	args.buffer->end = args.buffer->begin + have_size;
    }

    *args.buffer->end = '\0';

    return retval;
    }*/

keyargs_define(buffer_write)
{
    assert (args.buffer);
    assert (args.wrote_size);

    const char * write_point = args.buffer->begin + *args.wrote_size;

    if (write_point >= args.buffer->end)
    {
	assert (write_point == args.buffer->end);
	return 0;
    }

    ssize_t retval = write (args.fd, write_point, args.buffer->end - write_point);

    if (retval >= 0)
    {
	*args.wrote_size += retval;
    }

    return retval;
}

long int buffer_printf(buffer_char * buffer, const char * str, ...)
{
    va_list list;
    va_start(list, str);
    size_t len = vsnprintf(NULL, 0, str, list);
    va_end(list);

    size_t new_size = len + 1;

    buffer_realloc(*buffer, new_size); 

    va_start(list, str);
    vsprintf(buffer->begin, str, list);
    va_end(list);

    buffer->end = buffer->begin + len;
    *buffer->end = '\0';

    return len;
}

long int buffer_printf_append(buffer_char * buffer, const char * str, ...)
{
    va_list list;
    va_start(list, str);
    size_t len = vsnprintf(NULL, 0, str, list);
    va_end(list);

    size_t old_size = range_count(*buffer);

    size_t new_size = old_size + len + 1;

    buffer_realloc(*buffer, new_size); 

    va_start(list, str);
    vsprintf(buffer->begin + old_size, str, list);
    va_end(list);

    buffer->end = buffer->begin + new_size - 1;
    *buffer->end = '\0';

    return len;
}

inline static void buffer_downshift_protect (size_t protect_size, buffer_char * buffer, char * new_begin)
{
    range_char delete = { .begin = buffer->begin + protect_size, .end = new_begin };

    if (delete.begin >= buffer->end)
    {
	assert (range_count (*buffer) >= 0);
	return;
    }

    if (delete.end >= buffer->end)
    {
	buffer->end = delete.begin;
	assert (range_count (*buffer) >= 0);
	return;
    }

    size_t shift_size = buffer->end - delete.end;

    assert (shift_size);

    memmove (delete.begin, delete.end, shift_size);

    buffer->end -= range_count (delete);

    assert (range_count (*buffer) >= 0);
}

keyargs_define (buffer_getline_fd)
{
    int size;
    char * next_end;
    //int skip = args.protect_size;

    const char * sep = args.sep;
    if (!sep)
    {
	sep = "\n";
    }
    
    int sep_len = strlen (sep);
    
    if ((size_t) range_count (*args.read.buffer) <= args.protect_size)
    {
	//log_debug ("Out of bytes, getting more...");
	goto readnew;
    }

    if (args.init)
    {
	args.line->begin = args.read.buffer->begin + args.protect_size;
	args.line->end = strstr (args.line->begin, sep);

	if (args.line->end)
	{
	    //log_debug("Have end already: %zu", range_count(*args.line));
	    return true;
	}
	else
	{
	    //log_debug ("Searching for end...");
	    goto readnew;
	}
    }

//parseold:
    //assert (args.line->end < args.read.buffer->end - sep_len);

    args.line->begin = args.line->end + sep_len;

    if (args.line->begin > args.read.buffer->end)
    {
	return false;
    }

    //log_debug ("Finding: %s", args.line->begin);
    next_end = strstr (args.line->begin, sep);

    if (next_end)
    {
	args.line->end = next_end;
	assert (args.line->end < args.read.buffer->end);
	return true;
    }
    
//shiftdown:
    //log_debug ("shiftdown");
    buffer_downshift_protect (args.protect_size, args.read.buffer, args.line->end + sep_len);

readnew:
    while (0 < (size = keyargs_func_name(buffer_read) (args.read)))
    {
	if (size < 0)
	{
	    log_error ("Error reading from file descriptor");
	    return false;
	}

	if (size == 0)
	{
	    break;
	}

	//log_debug ("read message: [%.*s]", (int) range_count (*args.read.buffer) - args.protect_size, args.read.buffer->begin + args.protect_size);

	args.line->end = strstr (args.read.buffer->begin + args.protect_size, sep);
	assert (args.line->end < args.read.buffer->end);

	if (args.line->end)
	{
	    args.line->begin = args.read.buffer->begin + args.protect_size;
	    //log_debug ("set begin: [%s]", args.line->begin);
	    return true;
	}

	//skip = args.protect_size + range_count (*args.read.buffer);
    }
    
    if ((size_t) range_count (*args.read.buffer) <= args.protect_size)
    {
	return false;
    }

    args.line->begin = args.read.buffer->begin + args.protect_size;
    args.line->end = args.read.buffer->end;

    return true;
}

void buffer_getline_end (size_t protect_size, const char * sep, range_char * line, buffer_char * buffer)
{
    buffer_downshift_protect (protect_size, buffer, line->end + strlen(sep));
}
