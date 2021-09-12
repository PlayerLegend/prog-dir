//#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "dzip.h"
#include "../buffer_io/buffer_io.h"

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

long long int dzip_inflate_range (buffer_unsigned_char * output, range_const_unsigned_char * input)
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
	    return -1;
	}

	if (chunk.cast->header.version != DZIP_VERSION)
	{
	    return -1;
	}

	if (!dzip_inflate(.chunk = chunk.cast,
			  .output = output))
	{
	    return -1;
	}

	chunk.byte += chunk.cast->header.chunk_size;
    }

    return chunk.byte - input->begin;
}
