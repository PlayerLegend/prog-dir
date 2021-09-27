//#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../keyargs/keyargs.h"
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../../chain-io/common.h"
#include "../../chain-io/read.h"
#include "../../chain-io/write.h"
#include "../common/common.h"
#include "inflate.h"
#include "../../buffer_io/buffer_io.h"
#include "extensions.h"

#include "../../log/log.h"

bool dzip_inflate_read_chunk (buffer_unsigned_char * chunk, int fd)
{
    buffer_rewrite (*chunk);
    long int size;

    while (0 < (size = buffer_read (.buffer = &chunk->char_cast,
				    .max_buffer_size = sizeof(dzip_header),
				    .fd = fd)))
    {}

    if (size < 0)
    {
	return false;
    }

    if (range_is_empty (*chunk))
    {
	return true;
    }

    const unsigned char magic[] = DZIP_MAGIC_INITIALIZER;

    if (0 != memcmp (magic, ((dzip_header*) chunk->begin)->magic, sizeof(magic)))
    {
	return false;
    }

    if (DZIP_VERSION != ((dzip_header*) chunk->begin)->version)
    {
	return false;
    }

    dzip_size chunk_size = ((dzip_header*) chunk->begin)->chunk_size;

    if (chunk_size < sizeof(dzip_header))
    {
	return false;
    }

    while (0 < (size = buffer_read (.buffer = &chunk->char_cast,
				    .max_buffer_size = chunk_size,
				    .fd = fd)))
    {}

    if (range_count(*chunk) != chunk_size)
    {
	return false;
    }

    return true;
}

bool dzip_inflate_range (buffer_unsigned_char * output, range_const_unsigned_char * input)
{
    union {
	const dzip_chunk * cast;
	const unsigned char * byte;
    }
	chunk = { .byte = input->begin };

    const unsigned char magic[] = DZIP_MAGIC_INITIALIZER;

    while (chunk.byte < input->end - sizeof(*chunk.cast))
    {
	if (0 != memcmp (chunk.cast->header.magic, magic, sizeof(magic)))
	{
	    return false;
	}

	if (chunk.cast->header.version != DZIP_VERSION)
	{
	    return false;
	}

	if (chunk.byte + chunk.cast->header.chunk_size > input->end)
	{
	    break;
	}

	if (!dzip_inflate(.chunk = chunk.cast,
			  .output = output))
	{
	    return false;
	}

	chunk.byte += chunk.cast->header.chunk_size;
    }

    input->begin = chunk.byte;

    return true;
}

/*
typedef struct {
    io_read * source;
    dzip_deflate_state * state;
}
    dzip_deflate_io_read_arg;

static io_status io_read_dzip_update_func (buffer_unsigned_char * buffer, void * arg_orig)
{
    dzip_deflate_io_read_arg * arg = arg_orig;

    io_status status = io_read_update (arg->source);

    if (status == IO_ERROR)
    {
	return IO_ERROR;
    }

    range_const_unsigned_char contents = {0};

    io_read_contents(&contents, arg->source);

    if (status == IO_COMPLETE || range_count (contents) > 1e6)
    {
	dzip_deflate(.state = arg->state,
		     .input = &contents,
		     .output = buffer);

	io_read_release(arg->source, range_count (contents));
    }

    return status;
}

static void io_read_dzip_free_arg (void * arg_orig)
{
    dzip_deflate_io_read_arg * arg = arg_orig;
    dzip_deflate_state_free(arg->state);
    free (arg);
}

io_read * io_read_open_dzip (io_read * source)
{
    dzip_deflate_io_read_arg * arg = malloc (sizeof(*arg));
    arg->source = source;
    arg->state = dzip_deflate_state_new();

    return io_read_new (.func = io_read_dzip_update_func,
			.arg = arg,
			.free_arg = io_read_dzip_free_arg);
}
*/
