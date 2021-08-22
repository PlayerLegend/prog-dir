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
    size_t max_chunk_size;
    enum {
	STATE_HEADER_MAGIC,
	STATE_HEADER_SIZE,
	STATE_HEADER_WINDOW_SIZE,
	STATE_READING_CMD,
	STATE_READING_LITERAL_SIZE,
	STATE_READING_LITERAL_CONTENTS,
	STATE_READING_REPEAT_BYTE,
	STATE_READING_REPEAT_SIZE,
	STATE_READING_LZ77_DISTANCE,
	STATE_READING_LZ77_LENGTH,
	STATE_READING_LZ77_NEXT_CHAR,
    }
	state;
    
    size_t command_bytes_decoded;
    buffer_unsigned_char input;
    buffer_unsigned_char output;
    struct {
	struct {
	    vluint_result length;
	    size_t start;
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
    
    vluint_result window_size;
    size_t bytes_read;
    size_t input_shift_total;
    size_t output_shift_total;
    bool input_is_finished;
    vluint_result header_size;
    size_t header_start;
    size_t output_released;
};

keyargs_define(dzip_inflate_new)
{
    dzip_inflate_state * retval = calloc (1, sizeof(*retval));

    retval->max_chunk_size = args.max_chunk_size ? args.max_chunk_size : -1;

    return retval;
}

#define input_from_state(state_p) (range_const_unsigned_char) { .begin = (state)->input.begin + (state)->bytes_read - (state)->input_shift_total, .end = (state)->input.end }

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

void dzip_inflate_release_output (dzip_inflate_state * state, size_t size)
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
    size_t take;

    size_t initial_size;
    
    unsigned char * lz77_i;

    range_unsigned_char lz77_write;
    
    const range_const_unsigned_char input = input_from_state(state);

    assert (range_count (input) >= 0);

    if (range_is_empty (input))
    {
	if (state->input_is_finished && state->state != STATE_READING_CMD)
	{
	    log_fatal ("Compressed input is truncated at state %zu offset %zu", state->state, state->bytes_read);
	}
	return false;
    }

    //print_int ("mb read", state->bytes_read / (1024 * 1024));
    //log_debug ("state %d, offset %zu", state->state, state->bytes_read);
    
    switch (state->state)
    {
    case STATE_HEADER_MAGIC:
	if ((size_t)range_count (input) < sizeof (DZIP_MAGIC))
	{
	    return false;
	}

	if (*(uint64_t*)input.begin != DZIP_MAGIC)
	{
	    log_fatal ("Input stream magic is not dzip");
	}

	state->bytes_read += sizeof(DZIP_MAGIC);
	state->state = STATE_HEADER_SIZE;

	return true;

    case STATE_HEADER_SIZE:
	if (!vluint_read (.result = &state->header_size, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->bytes_read += range_count (vluint);
	state->header_start = state->bytes_read;
	state->state = STATE_HEADER_WINDOW_SIZE;

	return true;

    case STATE_HEADER_WINDOW_SIZE:
	if (!vluint_read (.result = &state->window_size, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->bytes_read += range_count (vluint);
	
	if (state->bytes_read - state->header_start != state->header_size)
	{
	    log_fatal ("Malformed header in dzip stream, header size is incorrect with %zu(claimed) != %zu(actual)", state->header_size, state->bytes_read - state->header_start);
	}

	state->state = STATE_READING_CMD;

	return true;

    case STATE_READING_CMD:
	
	state->bytes_read++;

	switch (*input.begin)
	{
	case DZIP_CMD_LITERAL:
	    state->state = STATE_READING_LITERAL_SIZE;
	    return true;

	case DZIP_CMD_REPEAT:
	    state->state = STATE_READING_REPEAT_BYTE;
	    return true;

	case DZIP_CMD_LZ77:
	    state->state = STATE_READING_LZ77_DISTANCE;
	    return true;

	default:
	    log_fatal ("Invalid command in dzip stream: %d", *input.begin);
	}

	assert (false);
	return false;

    case STATE_READING_LITERAL_SIZE:
	if (!vluint_read (.result = &state->command_parameters.literal.length, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->bytes_read += range_count (vluint);
	state->command_parameters.literal.start = state->bytes_read;
	state->state = STATE_READING_LITERAL_CONTENTS;

	//log_debug ("read literal vluint size %zu", range_count (vluint));

	return true;

    case STATE_READING_LITERAL_CONTENTS:
	assert (state->bytes_read >= state->command_parameters.literal.start);
	assert (state->command_parameters.literal.length >= (state->bytes_read - state->command_parameters.literal.start));
	
	take = state->command_parameters.literal.length - (state->bytes_read - state->command_parameters.literal.start);

	assert (take > 0);

	if (take > (size_t)range_count (input))
	{
	    take = range_count (input);
	}
	else
	{
	    state->state = STATE_READING_CMD;
	}

	buffer_append_n (state->output, input.begin, take);

	state->bytes_read += take;

	//log_debug ("wrote literal %zu", state->command_parameters.literal.length);

	state->state = STATE_READING_CMD;
	
	return true;

    case STATE_READING_LZ77_DISTANCE:
	if (!vluint_read (.result = &state->command_parameters.lz77.match.distance, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->bytes_read += range_count (vluint);
	state->state = STATE_READING_LZ77_LENGTH;

	return true;
	
    case STATE_READING_LZ77_LENGTH:
	if (!vluint_read (.result = &state->command_parameters.lz77.match.length, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->bytes_read += range_count (vluint);
	state->state = STATE_READING_LZ77_NEXT_CHAR;

	return true;

    case STATE_READING_LZ77_NEXT_CHAR:

	state->command_parameters.lz77.match.next_char = *input.begin;
	state->bytes_read++;

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

    case STATE_READING_REPEAT_BYTE:

	state->command_parameters.repeat.byte = *input.begin;
	state->bytes_read++;

	state->state = STATE_READING_REPEAT_SIZE;
	
	return true;
	
    case STATE_READING_REPEAT_SIZE:
	if (!vluint_read (.result = &state->command_parameters.repeat.length, .input = &input, .vluint = &vluint))
	{
	    goto vluint_incomplete;
	}

	state->bytes_read += range_count (vluint);

	//log_debug ("writing repeat %d[%zu]", state->command_parameters.repeat.byte, state->command_parameters.repeat.length);
	
	for (size_t i = 0; i < state->command_parameters.repeat.length; i++)
	{
	    *buffer_push (state->output) = state->command_parameters.repeat.byte;
	}

	state->state = STATE_READING_CMD;

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
