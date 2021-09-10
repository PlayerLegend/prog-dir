#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "dzip.h"
#include "internal.h"
#include "../log/log.h"
#include "../vluint/vluint.h"

struct dzip_inflate_state
{
    enum {
	INFLATE_READ_HEADER,
	INFLATE_READ_CMD,
	INFLATE_EXTEND_ARG1,
	INFLATE_SELECT_JOB,
	INFLATE_READ_LITERAL,
	INFLATE_READ_MATCH_POINT,
	INFLATE_READ_MATCH_CONTENTS,
    }
	job;

    unsigned char cmd;

    union {
	uint16_t arg1, literal_length, match_length;
    };
    
    union {
	vluint_result arg2, match_point;
    };
    
    dzip_size command_bytes, bytes_read, bytes_written;

    char magic[8]; 
};

dzip_inflate_state * dzip_inflate_state_new()
{
    return calloc (1, sizeof(dzip_inflate_state));
}

keyargs_define(dzip_inflate)
{
    char test_magic[] = DZIP_MAGIC_INITIALIZER;
    if (0 != memcmp (test_magic, args.chunk->header.magic, sizeof(test_magic)))
    {
	log_fatal ("Invalid magic in dzip chunk");
    }

    if (args.chunk->header.version != DZIP_VERSION)
    {
	log_fatal ("Incorrect version in dzip header");
    }

    assert (args.chunk->bytes == (const unsigned char *) args.chunk + sizeof(dzip_header));
    
    range_const_unsigned_char input = { .begin = args.chunk->bytes, .end = ((const unsigned char*) args.chunk) + args.chunk->header.chunk_size };

    if (range_count (input) <= 0)
    {
	log_fatal ("Invalid size in dzip header");
    }

    char cmd;
    
    vluint_result arg1;
    vluint_result arg2;

    dzip_size output_size;
    unsigned char copy;
    bool extend;

read_cmd:

    if (range_is_empty(input))
    {
	return true;
    }

    //log_debug ("reading cmd %zu at %zu", *input.begin, input.begin - (const unsigned char*)args.chunk);
    
    cmd = *input.begin & DZIP_COMMAND_MASK;
    arg1 = (*input.begin >> DZIP_META_BITS) & DZIP_ARG1_COMPACT_MASK;
    extend = *input.begin & DZIP_ARG1_EXTEND_BIT;
    
    input.begin++;

    if (extend)
    {
	goto read_extended_arg1;
    }
    else
    {
	goto cmd_switch;
    }

    assert(false);

read_extended_arg1:

    //log_debug ("extended");

    if (!vluint_read (.result = &arg2,
		      .input = &input,
		      .rest = &input))
    {
	log_fatal ("Truncated extended arg1");
    }
    
    arg1 += DZIP_ARG1_COMPACT_MAX * arg2;

    goto cmd_switch;

cmd_switch:

    //log_debug ("cmd switch %d %d", cmd, arg1);
    
    switch (cmd)
    {
    case DZIP_CMD_LITERAL: goto read_literal;
    case DZIP_CMD_MATCH: goto read_match;
    default: log_fatal ("Invalid dzip command");
    }

    assert (false);

read_literal:

    //log_debug ("literal");
    
    if (arg1 == 0)
    {
	log_fatal ("dzip literal length is zero");
    }

    if (arg1 > (dzip_size)range_count(input))
    {
	log_fatal ("dzip literal length is too long");
    }
    
    buffer_append_n (*args.output, input.begin, arg1);
    
    input.begin += arg1;

    goto read_cmd;

read_match:

    //log_debug ("match");
    
    if (arg1 == 0)
    {
	log_fatal ("dzip match length is zero");
    }

    if (!vluint_read(.result = &arg2,
		     .input = &input,
		     .rest = &input))
    {
	log_fatal ("dzip match distance vluint is truncated");
    }

    if (arg2 == 0)
    {
	log_fatal ("dzip match distance is zero");
    }

    output_size = range_count(*args.output);
    
    if (arg2 > output_size)
    {
	log_fatal ("dzip match distance is too large");
    }

    while (arg1 > 0)
    {
	copy = args.output->end[-arg2];
	*buffer_push(*args.output) = copy;
	arg1--;
    }

    goto read_cmd;
    
fail:
    return false;
}
