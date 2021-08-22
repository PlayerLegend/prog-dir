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

struct dzip_inflate_state {
    bool error;
    dzip_size max_chunk_size;
    enum {
	STATE_HEADER_MAGIC,
	STATE_HEADER_SIZE,
	STATE_HEADER_WINDOW_SIZE,
	STATE_READING_CMD,
	STATE_READING_LITERAL_SIZE,
	STATE_READING_LITERAL_CONTENTS,
	STATE_READING_REPEAT_SIZE,
	STATE_READING_REPEAT_BYTE,
	STATE_READING_LZ77_LENGTH,
	STATE_READING_LZ77_DISTANCE,
	STATE_READING_LZ77_NEXT_CHAR,
    }
	state;
    
    dzip_size command_bytes_decoded;
    buffer_unsigned_char input;
    buffer_unsigned_char output;
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
	dzip_size bytes_read;
    }
	stream;
    
    vluint_result window_size;
    dzip_size input_shift_total;
    dzip_size output_shift_total;
    bool input_is_finished;
    vluint_result header_size;
    dzip_size header_start;
    dzip_size output_released;
};

keyargs_define(dzip_inflate_new)
{
    dzip_inflate_state * retval = calloc (1, sizeof(*retval));

    retval->max_chunk_size = args.max_chunk_size ? args.max_chunk_size : -1;

    return retval;
}

#define input_from_state(state_p) (range_const_unsigned_char) { .begin = (state)->input.begin + (state)->stream.bytes_read - (state)->input_shift_total, .end = (state)->input.end }

buffer_unsigned_char * dzip_inflate_input_buffer (dzip_inflate_state * state)
{
    return &state->input;
}

void dzip_inflate_eof (dzip_inflate_state * state)
{
    state->input_is_finished = true;
}

/*static void print_int(const char * message, int n)
{
    static int number;

    if (number != n)
    {
	number = n;
	log_debug ("%s: %d", message,  n);
    }
    }*/

void dzip_inflate_release_output (dzip_inflate_state * state, dzip_size size)
{
    state->output_released += size;
}

void dzip_inflate_get_output (range_const_unsigned_char * output, dzip_inflate_state * state)
{
    assert (state->output_shift_total <= state->output_released);

    assert (range_count (state->output) >= 0);

    output->begin = state->output.begin + state->output_shift_total - state->output_released;
    output->end = state->output.end;

    assert (range_count(*output) >= 0);
}

bool dzip_inflate_update (dzip_inflate_state * state)
{
    range_const_unsigned_char vluint;
    dzip_size take;

    dzip_size initial_size;
    
    unsigned char * lz77_i;

    range_unsigned_char lz77_write;
    
    const range_const_unsigned_char input = input_from_state(state);

    unsigned char cmd;
    unsigned char arg;

    assert (range_count (input) >= 0);

    if (range_is_empty (input))
    {
	if (state->input_is_finished && state->state != STATE_READING_CMD)
	{
	    log_fatal ("Compressed input is truncated at state %zu offset %zu", state->state, state->stream.bytes_read);
	}
	return false;
    }

    //print_int ("mb read", state->bytes_read / (1024 * 1024));
    //log_debug ("state %d, offset %zu", state->state, state->bytes_read);
    
    switch (state->state)
    {
    case STATE_HEADER_MAGIC:
	if ((dzip_size)range_count (input) < sizeof (DZIP_MAGIC))
	{
	    return false;
	}

	if (*(uint64_t*)input.begin != DZIP_MAGIC)
	{
	    log_fatal ("Input stream magic is not dzip");
	}

	state->stream.bytes_read += sizeof(DZIP_MAGIC);
	state->state = STATE_HEADER_SIZE;

	return true;

    case STATE_HEADER_SIZE:
	if (!vluint_read (.result = &state->header_size, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->stream.bytes_read += range_count (vluint);
	state->header_start = state->stream.bytes_read;
	state->state = STATE_HEADER_WINDOW_SIZE;

	return true;

    case STATE_HEADER_WINDOW_SIZE:
	if (!vluint_read (.result = &state->window_size, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->stream.bytes_read += range_count (vluint);
	
	if (state->stream.bytes_read - state->header_start != state->header_size)
	{
	    log_fatal ("Malformed header in dzip stream, header size is incorrect with %zu(claimed) != %zu(actual)", state->header_size, state->stream.bytes_read - state->header_start);
	}

	state->state = STATE_READING_CMD;

	return true;

    case STATE_READING_CMD:
	
	state->stream.bytes_read++;

	read_command(&cmd, &arg, *input.begin);

	switch (cmd)
	{
	case DZIP_CMD_LITERAL:
	    if (arg)
	    {
		//log_debug ("compact literal");
		state->command_parameters.literal.length = arg;
		state->command_parameters.literal.start = state->stream.bytes_read;
		state->state = STATE_READING_LITERAL_CONTENTS;
	    }
	    else
	    {
		state->state = STATE_READING_LITERAL_SIZE;
	    }
	    return true;

	case DZIP_CMD_REPEAT:
	    if (arg)
	    {
		//log_debug ("compact repeat");
		state->command_parameters.repeat.length = arg;
		state->state = STATE_READING_REPEAT_BYTE;
	    }
	    else
	    {
		state->state = STATE_READING_REPEAT_SIZE;
	    }
	    return true;

	case DZIP_CMD_LZ77:
	    if (arg)
	    {
		//log_debug ("compact lz77");
		state->command_parameters.lz77.match.length = arg;
		state->state = STATE_READING_LZ77_DISTANCE;
	    }
	    else
	    {
		state->state = STATE_READING_LZ77_LENGTH;
	    }
	    return true;

	default:
	    log_fatal ("Invalid command in dzip stream: %d:%d", cmd, arg);
	}

	assert (false);
	return false;

    case STATE_READING_LITERAL_SIZE:
	if (!vluint_read (.result = &state->command_parameters.literal.length, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	//log_debug ("got literal size %zu", state->command_parameters.literal.length);
	assert (state->command_parameters.literal.length > 0);

	state->stream.bytes_read += range_count (vluint);
	state->command_parameters.literal.start = state->stream.bytes_read;
	
	state->state = STATE_READING_LITERAL_CONTENTS;

	//log_debug ("read literal vluint size %zu", range_count (vluint));

	return true;

    case STATE_READING_LITERAL_CONTENTS:
	assert (state->stream.bytes_read >= state->command_parameters.literal.start);
	assert (state->command_parameters.literal.length >= (state->stream.bytes_read - state->command_parameters.literal.start));
	
	take = state->command_parameters.literal.length - (state->stream.bytes_read - state->command_parameters.literal.start);

	assert (take > 0);

	if (take > (dzip_size)range_count (input))
	{
	    take = range_count (input);
	}
	else
	{
	    state->state = STATE_READING_CMD;
	}

	buffer_append_n (state->output, input.begin, take);

	state->stream.bytes_read += take;

	//log_debug ("read literal %zu", state->command_parameters.literal.length);

	state->state = STATE_READING_CMD;
	
	return true;

    case STATE_READING_LZ77_LENGTH:
	if (!vluint_read (.result = &state->command_parameters.lz77.match.length, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->stream.bytes_read += range_count (vluint);
	state->state = STATE_READING_LZ77_DISTANCE;

	return true;

    case STATE_READING_LZ77_DISTANCE:
	if (!vluint_read (.result = &state->command_parameters.lz77.match.distance, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->stream.bytes_read += range_count (vluint);
	state->state = STATE_READING_LZ77_NEXT_CHAR;

	return true;
	
    case STATE_READING_LZ77_NEXT_CHAR:

	state->command_parameters.lz77.match.next_char = *input.begin;
	state->stream.bytes_read++;

	initial_size = range_count (state->output);
	buffer_resize (state->output, initial_size + state->command_parameters.lz77.match.length);

	lz77_write.end = state->output.end;
	lz77_write.begin = lz77_write.end - state->command_parameters.lz77.match.length;

	if (state->command_parameters.lz77.match.distance > state->window_size)
	{
	    log_fatal ("lz77 triple length exceeds window size");
	}

	if (lz77_write.begin - state->command_parameters.lz77.match.distance < state->output.begin)
	{
	    log_fatal ("Output is smaller than the minimum required for a lz77 triple offset");
	}

	for_range (lz77_i, lz77_write)
	{
	    *lz77_i = *(lz77_i - state->command_parameters.lz77.match.distance);
	}

	//log_debug ("wrote lz77 match (%zu, %zu, %d)", state->command_parameters.lz77.match.distance, state->command_parameters.lz77.match.length, state->command_parameters.lz77.match.next_char);

	state->state = STATE_READING_CMD;
	
	return true;

    case STATE_READING_REPEAT_SIZE:
	if (!vluint_read (.result = &state->command_parameters.repeat.length, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->stream.bytes_read += range_count (vluint);

	state->state = STATE_READING_REPEAT_BYTE;

	return true;
	
    case STATE_READING_REPEAT_BYTE:

	state->command_parameters.repeat.byte = *input.begin;
	state->stream.bytes_read++;

	for (dzip_size i = 0; i < state->command_parameters.repeat.length; i++)
	{
	    *buffer_push (state->output) = state->command_parameters.repeat.byte;
	}

	state->state = STATE_READING_CMD;

	//log_debug ("read repeat %zu[%d]",
	//state->command_parameters.repeat.length,
	//	   state->command_parameters.repeat.byte);

	return true;
    }

vluint_incomplete:
    if (!state->input_is_finished)
    {
	return false;
    }
    else
    {
	log_fatal ("Incomplete vluint in state %zu", state->state);
    }

fail:
    state->error = true;
    return false;
}
