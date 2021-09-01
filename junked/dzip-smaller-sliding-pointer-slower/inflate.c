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

struct dzip_inflate_state
{
    dzip_window window;
    enum {
	INFLATE_JOB_READ_HEADER,
	INFLATE_JOB_READ_CMD,
	INFLATE_JOB_READ_ARG1,
	INFLATE_JOB_READ_LITERAL,
	INFLATE_JOB_READ_MATCH_POINT,
    }
	job;

    struct {
	size_t point;
	union {
	    char c[2];
	    uint16_t i;
	};
    }
	arg1;

    struct {
	char bytes[8];
	unsigned char point;
    }
	metadata_tmp;
};

dzip_inflate_state * dzip_inflate_state_new()
{
    return calloc (1, sizeof(dzip_inflate_state));
}

inline static bool metadata_tmp_get (dzip_inflate_state * state, range_const_char * input, unsigned char want)
{
    while (state->metadata_tmp.point < want && input->begin < input->end)
    {
	state->metadata_tmp.bytes[state->metadata_tmp.point++] = *(input->begin++);
    }

    if (state->metadata_tmp.point == want)
    {
	state->metadata_tmp.point = 0;
	return true;
    }
    else
    {
	return false;
    }
}

keyargs_define(dzip_inflate)
{
    range_const_char input = *args.input;

next:
#define goto_next(job_id) { args.state->job = job_id; goto job_id; }
    
    switch (args.state->job)
    {
    case INFLATE_JOB_READ_HEADER:
	if (metadata_tmp_get (args.state, &input, 8))
	{
	    if (*(uint64_t*)args.state->metadata_tmp.bytes != *(uint64_t*)"deezwhat")
	    {
		log_fatal ("Invalid header magic in stream - this is not a dzip stream");
	    }
	    else
	    {
		goto_next(INFLATE_JOB_READ_CMD);
	    }
	}
	else
	{
	    goto need_more_bytes;
	}
	assert (false);
        
    case INFLATE_JOB_READ_CMD:	
    INFLATE_JOB_READ_CMD:
	if (metadata_tmp_get(args.state, &input, 1))
	{
	    args.state->arg1.c[args.state->arg1.point] = args.state->metadata_tmp.bytes[0];

	    if (0 == args.state->arg1.point)
	    {

		
	    }
	}
	else
	{
	    goto need_more_bytes;
	}
	assert (false);
    }

need_more_bytes:
    return true;

fail:
    return false;
}
