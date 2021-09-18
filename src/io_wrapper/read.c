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
#include "read.h"

struct io_read {
    io_read_update_func func;
    void * arg;
    size_t released;
    void (*free_arg)(void * arg);
    buffer_unsigned_char buffer;
};

#define MAX_BUFFER_SIZE ((size_t)1e7)

keyargs_define(io_read_new)
{
    io_read * io = calloc (1, sizeof(*io));

    io->func = args.func;
    io->arg = args.arg;
    io->free_arg = args.free_arg;

    return io;
}

static void io_read_apply_release (io_read * io)
{
    size_t buffer_size = range_count (io->buffer);

    if (io->released >= buffer_size)
    {
	buffer_rewrite (io->buffer);
	io->released = 0;
    }
    else if (io->released > buffer_size / 2 || buffer_size > MAX_BUFFER_SIZE)
    {
	buffer_downshift (io->buffer, io->released);
	io->released = 0;
    }
}

io_status io_read_update (io_read * io)
{
    io_read_apply_release (io);
    
    size_t start_size = range_count (io->buffer);
    size_t max_size = start_size < MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : start_size + MAX_BUFFER_SIZE;

    io_status status = IO_ERROR;
    
    while (true)
    {
	status = io->func (&io->buffer, io->arg);
	
	if (status != IO_INCOMPLETE)
	{
	    return status;
	}

	if (max_size < (size_t) range_count (io->buffer))
	{
	    return IO_INCOMPLETE;
	}
    }
}

void io_read_contents (range_const_unsigned_char * contents, io_read * io)
{
    contents->begin = io->buffer.begin + io->released;
    contents->end = io->buffer.end;

    if (contents->begin > contents->end)
    {
	contents->begin = contents->end;
    }
}

void io_read_release(io_read * io, size_t size)
{
    io->released += size;
}

void io_read_free (io_read * io)
{
    free (io->buffer.begin);
    if (io->free_arg)
    {
	io->free_arg (io->arg);
    }
    free (io);
}
