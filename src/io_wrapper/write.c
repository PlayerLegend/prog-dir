#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#include "write.h"
#include "../libc/string.h"

struct io_write {
    io_write_update_func func;
    void * arg;
    size_t written;
    void (*free_arg)(void * arg);
    buffer_unsigned_char buffer;
    size_t flush_size;
};

#define BUFFER_MAX_SIZE 1e7
#define INPUT_FLUSH_SIZE 1e6

keyargs_define(io_write_new)
{
    io_write * io = calloc (1, sizeof(*io));

    io->func = args.func;
    io->arg = args.arg;
    io->free_arg = args.free_arg;
    io->flush_size = args.flush_size;

    return io;
}

static bool io_write_flush_input (io_write * io, range_const_unsigned_char * input)
{
    io_status status = IO_ERROR;
    
    while (true)
    {
	status = io->func (input, io->arg);

	if (status != IO_INCOMPLETE)
	{
	    if (status == IO_ERROR)
	    {
		return false;
	    }
	    else
	    {
		assert (status == IO_COMPLETE);
		return true;
	    }
	}
    }
}

bool io_write_update (io_write * io, const range_const_unsigned_char * input)
{
    if (io->written)
    {
	size_t buffer_size = range_count (io->buffer);
	
	if (io->written > buffer_size / 2 || buffer_size > BUFFER_MAX_SIZE)
	{
	    buffer_downshift (io->buffer, io->written);
	    io->written = 0;
	}
    }

    assert ((size_t)range_count (io->buffer) >= io->written);
    
    if ((size_t)range_count (io->buffer) == io->written && (size_t)range_count (*input) > io->flush_size)
    {
	range_const_unsigned_char input_modify = *input;
	
	if (io_write_flush_input(io, &input_modify))
	{
	    assert (io->written == (size_t)range_count (io->buffer));
	    buffer_append (io->buffer, input_modify);
	    return true;
	}
	else
	{
	    return false;
	}
    }
    else
    {
	buffer_append (io->buffer, *input);
	
	if (range_count(io->buffer) - io->written < INPUT_FLUSH_SIZE)
	{
	    return true;
	}
	else
	{
	    return io_write_flush (io);
	}
    }
}

bool io_write_flush(io_write * io)
{
    range_const_unsigned_char input =
	{ .begin = io->buffer.range_cast.const_cast.begin + io->written,
	  .end = io->buffer.range_cast.const_cast.end, };

    if (io_write_flush_input(io, &input))
    {
	io->written = input.begin - io->buffer.begin;
	return true;
    }
    else
    {
	return false;
    }
}


void io_write_free (io_write * io)
{
    io_write_flush (io);
    free (io->buffer.begin);
    if (io->free_arg)
    {
	io->free_arg (io->arg);
    }
    free (io);
}
