#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "dzip.h"
#include "internal.h"
#include "../log/log.h"

typedef struct {
    dzip_window window;
}
    dzip_past;

struct dzip_inflate_state
{
    dzip_past past;
    
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
	dzip_window_point arg2, match_point;
	unsigned char arg2_bytes[sizeof(dzip_window_point)];
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
    range_const_char input = *args.input;
    char copy_byte;

#define goto_next(job_id)			\
    {	/*log_debug("Job: %zu [%zu]", job_id, args.state->bytes_read + 1);*/ \
	input.begin++;				\
	args.state->bytes_read++;		\
	if (input.begin < input.end)		\
	{					\
	    goto job_id;			\
	}					\
	else					\
	{					\
	    args.state->job = job_id;		\
	    goto need_more_bytes;		\
	}					\
    }

    switch (args.state->job)
    {
    case INFLATE_READ_HEADER:
    INFLATE_READ_HEADER:

	if (args.state->bytes_read > 7)
	{
	    log_fatal ("Header reading code started after header position in stream");
	}
	else
	{
	    args.state->magic[args.state->bytes_read] = *input.begin;

	    if (args.state->bytes_read == 7)
	    {
		if (*(uint64_t*) args.state->magic != *(uint64_t*) "deezwhat")
		{
		    log_fatal ("Invalid header magic in stream - this is not a dzip stream");
		}
		else
		{
		    goto_next(INFLATE_READ_CMD);
		}
	    }
	}

	goto_next(INFLATE_READ_HEADER);
	assert(false);
	
    case INFLATE_READ_CMD:
    INFLATE_READ_CMD:
	args.state->cmd = *input.begin & DZIP_COMMAND_MASK;
	args.state->arg1 = (*input.begin >> DZIP_META_BITS) & DZIP_ARG1_COMPACT_MASK;
	
	assert (args.state->arg1 < DZIP_ARG1_COMPACT_MAX);

	if (*input.begin & DZIP_ARG1_EXTEND_BIT)
	{
	    //log_debug ("Read command extended: %zu %zu", args.state->cmd, args.state->arg1);
	    goto_next(INFLATE_EXTEND_ARG1);
	}
	else
	{
	    //log_debug ("Read command(%zu): %zu %zu [%zu] [%zu]", *input.begin, args.state->cmd, args.state->arg1, args.state->bytes_read, input.begin - args.input->begin);
	    goto INFLATE_SELECT_JOB;
	}
	assert (false);

    case INFLATE_EXTEND_ARG1:
    INFLATE_EXTEND_ARG1:
	//{ log_debug ("extended: %u + (%u)(%u) = %u + %u = %u", args.state->arg1, DZIP_ARG1_COMPACT_MAX, (unsigned char)*input.begin, (unsigned short)(DZIP_ARG1_COMPACT_MAX * *input.begin)); }
	args.state->arg1 += DZIP_ARG1_COMPACT_MAX * (unsigned char)*input.begin;
	//log_debug ("extended: %zu", args.state->arg1);
	assert (args.state->arg1 < DZIP_ARG1_EXTEND_MAX);
	//goto_next(INFLATE_SELECT_JOB);
	goto INFLATE_SELECT_JOB;
	assert (false);

    case INFLATE_SELECT_JOB:
    INFLATE_SELECT_JOB:
	args.state->command_bytes = 0;

	if (args.state->arg1 == 0)
	{
	    log_fatal ("Command has null arg1");
	}

	if (args.state->cmd == DZIP_CMD_LITERAL)
	{
	    //log_debug ("reading literal: %zu", args.state->literal_length);
	    goto_next (INFLATE_READ_LITERAL);
	}
	else if (args.state->cmd == DZIP_CMD_MATCH)
	{
	    //log_debug ("reading match: %zu", args.state->match_length);
	    goto_next (INFLATE_READ_MATCH_POINT);
	}
	else
	{
	    log_fatal("Invalid command in dzip stream");
	}
	assert (false);

    case INFLATE_READ_LITERAL:
    INFLATE_READ_LITERAL:
	
	reference_window_byte(args.state->past.window, args.state->bytes_written) = *input.begin;
	*buffer_push (*args.output) = *input.begin;
	args.state->bytes_written++;
	args.state->command_bytes++;
	//log_debug ("literal[%zu]: %zu", args.state->bytes_read, (unsigned char)*input.begin);

	if (args.state->command_bytes == args.state->literal_length)
	{
	    goto_next (INFLATE_READ_CMD);
	}
	else
	{
	    assert (args.state->command_bytes < args.state->literal_length);
	    goto_next (INFLATE_READ_LITERAL);
	}
	assert (false);

    case INFLATE_READ_MATCH_POINT:
    INFLATE_READ_MATCH_POINT:

	if (args.state->command_bytes >= sizeof(args.state->arg2))
	{
	    assert (false);
	    log_fatal ("A bug has occurred while reading match point");
	}

	args.state->arg2_bytes[args.state->command_bytes++] = *input.begin;

	if (args.state->command_bytes == sizeof(args.state->arg2))
	{
	    args.state->command_bytes = 0;
	    if (args.state->match_point >= count_array(args.state->past.window))
	    {
		log_fatal ("Match point exceeds window length");
	    }
	    //goto_next(INFLATE_READ_MATCH_CONTENTS);
	    goto INFLATE_READ_MATCH_CONTENTS;
	}
	else
	{
	    goto_next(INFLATE_READ_MATCH_POINT);
	}
	assert (false);

    case INFLATE_READ_MATCH_CONTENTS:
    INFLATE_READ_MATCH_CONTENTS:
	//log_debug ("copy %zu/%zu", args.state->command_bytes, args.state->match_length);
	copy_byte = reference_window_byte(args.state->past.window, args.state->match_point + args.state->command_bytes);
	reference_window_byte(args.state->past.window, args.state->bytes_written) = copy_byte;
	*buffer_push (*args.output) = copy_byte;
	args.state->bytes_written++;
	args.state->command_bytes++;

	if (args.state->command_bytes == args.state->match_length)
	{
	    goto_next(INFLATE_READ_CMD);
	}
	else
	{
	    goto INFLATE_READ_MATCH_CONTENTS;
	}
	assert (false);
    }
    
need_more_bytes:
    return true;

fail:
    return false;
}
