#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "dzip.h"
#include "../vluint/vluint.h"
#include "internal-shared.h"
#include "../log/log.h"
#include "../buffer_stream/buffer_stream.h"

struct dzip_inflate_state {
    bool error;
    enum {
	STATE_HEADER_MAGIC,
	STATE_HEADER_SIZE,
	STATE_HEADER_WINDOW_SIZE,
	STATE_READING_CMD,
	STATE_READING_LITERAL_SIZE,
	STATE_READING_LITERAL_CONTENTS,
	STATE_READING_LZ77_LENGTH,
	STATE_READING_LZ77_DISTANCE,
    }
	state;
    
    struct {
	struct {
	    vluint_result length;
	    dzip_size start;
	}
	    literal;

	struct {
	    dzip_match match;
	}
	    lz77;

	struct {
	    unsigned char byte;
	    vluint_result length;
	}
	    repeat;
    }
	command_parameters;

    struct {
	dzip_size output_released;
	dzip_size bytes_read;
	buffer_stream_unsigned_char input;
	buffer_stream_unsigned_char output;
	bool input_is_finished;
    }
	stream;

    struct {
	dzip_size header_start;
	vluint_result header_size;
	vluint_result window_size;
	dzip_size max_chunk_size;	
    }
	stream_parameters;
    
};

/*size_t count_dead_bytes (dzip_inflate_state * state)
{
    
}*/

keyargs_define(dzip_inflate_new)
{
    dzip_inflate_state * retval = calloc (1, sizeof(*retval));

    retval->stream_parameters.max_chunk_size = args.max_chunk_size ? args.max_chunk_size : -1;

    return retval;
}

buffer_unsigned_char * dzip_inflate_input_buffer (dzip_inflate_state * state)
{
    return &state->stream.input.buffer_cast;
}

void dzip_inflate_eof (dzip_inflate_state * state)
{
    state->stream.input_is_finished = true;
}

void dzip_inflate_release_output (dzip_inflate_state * state, dzip_size size)
{
    state->stream.output_released += size;
}

void dzip_inflate_get_output (range_const_unsigned_char * output, dzip_inflate_state * state)
{
    assert (state->stream.output.shift <= state->stream.output_released);

    assert (range_count (state->stream.output) >= 0);

    output->begin = buffer_stream_pointer(state->stream.output, state->stream.output_released);
    output->end = state->stream.output.end;

    assert (range_count(*output) >= 0);
}

inline static void prune_input (dzip_inflate_state * state)
{
    assert (state->stream.bytes_read >= state->stream.input.shift);
    
    size_t dead_bytes = state->stream.bytes_read - state->stream.input.shift;

    if (dead_bytes > 1e5)// && dead_bytes > (size_t)range_count (state->stream.input) / 2)
    {
	log_debug ("prune %zu", dead_bytes);
	buffer_stream_shift (state->stream.input, dead_bytes);
    }
}

bool dzip_inflate_update (dzip_inflate_state * state)
{
    range_const_unsigned_char vluint;
    dzip_size literal_take;
    dzip_size literal_want;

    dzip_size initial_size;
    
    unsigned char * lz77_i;

    range_unsigned_char lz77_write;
    
    const range_const_unsigned_char input = { .begin = buffer_stream_pointer(state->stream.input, state->stream.bytes_read),
					      .end = state->stream.input.end };

    unsigned char cmd;
    unsigned char arg;

    assert (range_count (input) >= 0);

    if (range_is_empty (input))
    {
	if (state->stream.input_is_finished && state->state != STATE_READING_CMD)
	{
	    log_fatal ("Compressed input is truncated at state %zu offset %zu", state->state, state->stream.bytes_read);
	}

	goto incomplete;
    }

    //print_int ("mb read", state->bytes_read / (1024 * 1024));
    //log_debug ("state %d, offset %zu, remaining %zu", state->state, state->stream.bytes_read, state->stream.input.end - buffer_stream_pointer(state->stream.input, state->stream.bytes_read));

#define state_next(bytes,stateval) { state->stream.bytes_read += bytes; state->state = stateval; goto update; }
    
    switch (state->state)
    {
    case STATE_HEADER_MAGIC:
	if ((dzip_size)range_count (input) < sizeof (DZIP_MAGIC))
	{
	    goto incomplete;
	}

	if (*(uint64_t*)input.begin != DZIP_MAGIC)
	{
	    log_fatal ("Input stream magic is not dzip");
	}

	state_next (sizeof(DZIP_MAGIC), STATE_HEADER_SIZE);

    case STATE_HEADER_SIZE:
	if (!vluint_read (.result = &state->stream_parameters.header_size, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->stream_parameters.header_start = state->stream.bytes_read + range_count(vluint);

	state_next (range_count(vluint), STATE_HEADER_WINDOW_SIZE);

    case STATE_HEADER_WINDOW_SIZE:
	if (!vluint_read (.result = &state->stream_parameters.window_size, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	if (state->stream.bytes_read + range_count(vluint) - state->stream_parameters.header_start != state->stream_parameters.header_size)
	{
	    log_fatal ("Malformed header in dzip stream, header size is incorrect with %zu(claimed) != %zu(actual)", state->stream_parameters.header_size, state->stream.bytes_read - state->stream_parameters.header_start);
	}
        
	state_next (range_count(vluint), STATE_READING_CMD);
        
    case STATE_READING_CMD:
	
	//state->stream.bytes_read++;

	read_command(&cmd, &arg, *input.begin);

	switch (cmd)
	{
	case DZIP_CMD_LITERAL:
	    if (arg)
	    {
		//log_debug ("compact literal");
		state->command_parameters.literal.length = arg;
		state->command_parameters.literal.start = state->stream.bytes_read + 1;
		
		state_next (1, STATE_READING_LITERAL_CONTENTS);
	    }
	    else
	    {
		state_next (1, STATE_READING_LITERAL_SIZE);
	    }
	    
	case DZIP_CMD_LZ77:
	    if (arg)
	    {
		//log_debug ("compact lz77");
		state->command_parameters.lz77.match.length = arg;

		state_next (1, STATE_READING_LZ77_DISTANCE);
	    }
	    else
	    {
		state_next (1, STATE_READING_LZ77_LENGTH);
	    }

	default:
	    log_fatal ("Invalid command in dzip stream: %d:%d", cmd, arg);
	}

	assert (false);
	goto incomplete;

    case STATE_READING_LITERAL_SIZE:
	if (!vluint_read (.result = &state->command_parameters.literal.length, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	//log_debug ("got literal size %zu", state->command_parameters.literal.length);
	assert (state->command_parameters.literal.length > 0);

	//state->stream.bytes_read += range_count (vluint);
	state->command_parameters.literal.start = state->stream.bytes_read + range_count (vluint);
	
	//state->state = STATE_READING_LITERAL_CONTENTS;

	//log_debug ("read literal vluint size %zu value %zu", range_count (vluint), state->command_parameters.literal.length);

	//return true;

	state_next (range_count (vluint), STATE_READING_LITERAL_CONTENTS);

    case STATE_READING_LITERAL_CONTENTS:
	assert (state->stream.bytes_read >= state->command_parameters.literal.start);
	assert (state->command_parameters.literal.length >= (state->stream.bytes_read - state->command_parameters.literal.start));
	
	literal_want = state->command_parameters.literal.length - (state->stream.bytes_read - state->command_parameters.literal.start);

	assert (literal_want > 0);
	assert (literal_want <= state->command_parameters.literal.length);

	if (literal_want <= (size_t)range_count (input))
	{
	    literal_take = literal_want;
	}
	else
	{
	    literal_take = range_count (input);
	}
	
	//literal_take = literal_want < (dzip_size)range_count (input) ? literal_want : (dzip_size)range_count (input);

	/*
	if (take > (dzip_size)range_count (input))
	{
	    take = range_count (input);
	}
	else
	{
	    state->state = STATE_READING_CMD;
	    }*/

	buffer_append_n (state->stream.output, input.begin, literal_take);

	//state->stream.bytes_read += take;

	//log_debug ("read literal %zu", state->command_parameters.literal.length);

	//state->state = STATE_READING_CMD;
	
	//return true;

	//log_debug ("take/want/size %zu/%zu/%zu", literal_take, literal_want, state->command_parameters.literal.length);
	
	if (literal_take < literal_want)
	{
	    state_next (literal_take, STATE_READING_LITERAL_CONTENTS);
	}
	else
	{
	    assert (literal_take == literal_want);
	    state_next (literal_take, STATE_READING_CMD);
	}

    case STATE_READING_LZ77_LENGTH:
	if (!vluint_read (.result = &state->command_parameters.lz77.match.length, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state_next (range_count (vluint), STATE_READING_LZ77_DISTANCE);

    case STATE_READING_LZ77_DISTANCE:
	if (!vluint_read (.result = &state->command_parameters.lz77.match.distance, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	initial_size = range_count (state->stream.output);
	
	buffer_resize (state->stream.output, initial_size + state->command_parameters.lz77.match.length);

	lz77_write.end = state->stream.output.end;
	lz77_write.begin = lz77_write.end - state->command_parameters.lz77.match.length;

	if (state->command_parameters.lz77.match.distance > state->stream_parameters.window_size)
	{
	    log_fatal ("lz77 triple length exceeds window size");
	}

	if (lz77_write.begin - state->command_parameters.lz77.match.distance < state->stream.output.begin)
	{
	    log_fatal ("Output is smaller than the minimum required for a lz77 triple offset");
	}

	for_range (lz77_i, lz77_write)
	{
	    *lz77_i = *(lz77_i - state->command_parameters.lz77.match.distance);
	}

	//log_debug ("wrote lz77 match (%zu, %zu, %d)", state->command_parameters.lz77.match.distance, state->command_parameters.lz77.match.length, state->command_parameters.lz77.match.next_char);

	state_next (range_count(vluint), STATE_READING_CMD);
    }

    assert (false);

update:
    //log_debug ("u here");
    return true;
    
fail:
    state->error = true;
    return false;
    
vluint_incomplete:
    if (state->stream.input_is_finished)
    {
	log_fatal ("Incomplete vluint in state %zu", state->state);
    }

    goto incomplete;

incomplete:
    //log_debug ("i here");
    
    prune_input (state);
    
    return false;
}
