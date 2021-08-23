#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "dzip.h"
#include "../log/log.h"
#include "../vluint/vluint.h"

typedef enum {
    DZIP_CMD_LITERAL,
    DZIP_CMD_LZ77,
    DZIP_CMD_REPEAT,
}
    dzip_command;

struct dzip_deflate_state {
    size_t window_size;
    size_t max_match_length;
    size_t input_index;
    size_t literal_start;
    buffer_unsigned_char input;
    bool input_is_finished;
    size_t skip_count;
    struct {
	size_t match_count;
	size_t match_size;
	size_t match_offset;
	size_t longest_match;
	size_t literal_count;
	size_t literal_size;
    }
	stats;
};

typedef struct {
    size_t distance;
    size_t length;
    char next_char;
}
    dzip_match;

struct dzip_inflate_state {
    size_t max_chunk_size;
    dzip_command current_command;
    size_t command_bytes_decoded;
    struct {
	struct {
	    size_t length;
	}
	    literal;

	struct {
	    dzip_match match;
	}
	    lz77;

	struct {
	    unsigned char byte;
	    size_t length;
	}
	    repeat;
    }
	command_parameters;
};

keyargs_define(dzip_deflate_new)
{
    dzip_deflate_state * retval = calloc (1, sizeof(*retval));
    retval->window_size = args.window_size ? args.window_size : DZIP_DEFAULT_WINDOW_SIZE;
    retval->max_match_length = args.max_match_length ? args.max_match_length : retval->window_size;
    retval->skip_count = args.skip_count;
    
    //log_debug ("window size: %zu", retval->window_size);
    
    return retval;
}

keyargs_define(dzip_inflate_new)
{
    dzip_inflate_state * retval = calloc (1, sizeof(*retval));

    retval->max_chunk_size = args.max_chunk_size ? args.max_chunk_size : -1;

    return retval;
}

static void buffer_downshift (buffer_unsigned_char * buffer, size_t size)
{
    size_t buffer_size = range_count(*buffer);

    if (buffer_size <= size)
    {
	buffer_rewrite (*buffer);
	return;
    }
    
    size_t shift_size = buffer_size - size;

    assert (shift_size < buffer_size);
    assert (shift_size);

    memmove (buffer->begin, buffer->begin + size, shift_size);

    buffer->end -= size;

    assert ((size_t)range_count(*buffer) == shift_size);
}

/*static void print_int(const char * message, int n)
{
    static int number;

    if (number != n)
    {
	number = n;
	printf("%s: %d\n", message,  n);
    }
    }*/

void dzip_deflate_add (dzip_deflate_state * state, const range_const_unsigned_char * input)
{
    if (input)
    {
	buffer_append(state->input, *input);
    }
    else
    {
	state->input_is_finished = true;
    }
}

static size_t count_repeat (const range_const_unsigned_char * shift_input)
{
    unsigned char c = *shift_input->begin;

    const unsigned char * i;

    for_range (i, *shift_input)
    {
	if (*i != c)
	{
	    break;
	}
    }

    return range_index(i, *shift_input);
}

void write_literals (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    range_unsigned_char literal = { .begin = state->input.begin + state->literal_start,
				    .end = state->input.begin + state->input_index };

    if (!range_is_empty (literal))
    {
	//log_debug ("writing literal %zu", range_count (literal));
	    
	/*if (range_count (literal) > range_count(state->input))
	{
	    log_debug ("bad: %zd > %zd", range_count (literal), range_count(state->input));
	    exit(1);
	    }*/
	assert (range_count (literal) <= range_count(state->input));

	state->stats.literal_count++;
	state->stats.literal_size += range_count (literal);
	    
	*buffer_push (*output) = DZIP_CMD_LITERAL;
	vluint_write (output, range_count (literal));
	buffer_append (*output, literal);
	state->literal_start = state->input_index;
    }
}

inline static size_t count_dead_bytes (dzip_deflate_state * state)
{
    size_t count = 0;

    if (state->input_index > state->window_size)
    {
	count = state->input_index - state->window_size;
    }
    
    if (count > state->literal_start)
    {
	count = state->literal_start;
    }
    
    return count;
}

inline static void prune_dead_bytes (dzip_deflate_state * state)
{
    size_t dead_bytes = count_dead_bytes (state);

    if (dead_bytes > (size_t)range_count (state->input) / 2 && dead_bytes > 1e6)
    {
	assert (state->input.begin + state->input_index < state->input.end);
	buffer_downshift(&state->input, dead_bytes);
	state->input_index -= dead_bytes;
	state->literal_start -= dead_bytes;
	//log_debug ("trim");
	assert ((long long int)state->input_index < range_count (state->input));
    }
}

static size_t match_offset (dzip_match * match, dzip_deflate_state * state, const range_const_unsigned_char * input)
{
    *match = (dzip_match) { .next_char = state->input.begin[state->input_index] };

    size_t i_ofs;

    size_t max_ofs = state->input_index > state->window_size ? state->window_size : state->input_index;

    if (max_ofs == 0)
    {
	return 0;
    }

    const unsigned char * i_input;

    size_t test_length;

    range_const_unsigned_char scan = { .begin = input->begin, .end = scan.begin + state->max_match_length };

    if (scan.end > input->end - 1)
    {
	scan.end = input->end - 1;
    }

    //log_debug ("scanning for %zu * %zu = %zu", max_ofs - 1, range_count (scan), (max_ofs - 1) * range_count (scan));
    
    for (i_ofs = 1; i_ofs < max_ofs; i_ofs++)
    {
	for_range (i_input, scan)
	{
	    if (*i_input != *(i_input - i_ofs))
	    {
		break;
	    }
	}

	test_length = range_index(i_input, scan);

	if (test_length > match->length)
	{
	    match->length = test_length;
	    match->distance = i_ofs;
	    match->next_char = input->begin[test_length];
	}

	/*if (match->length && i_ofs > max_ofs / 4)
	{
	    break;
	    }*/
    }

    //log_debug ("length: %zu", match->length);
	    
    return match->length;
}

void dzip_write_header (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    assert (range_is_empty (*output));

    uint64_t magic = DZIP_MAGIC;
    
    buffer_append_n(*output, &magic, sizeof(magic));

    buffer_unsigned_char stream_metadata = {0};
    vluint_write(&stream_metadata, state->window_size);

    vluint_write(output, range_count(stream_metadata));
    buffer_append (*output, stream_metadata);
}

bool dzip_read_header (dzip_inflate_state * state)
{
    return false;
}

bool dzip_deflate_update (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    prune_dead_bytes (state);
    
    const range_const_unsigned_char input = { .begin = state->input.begin + state->input_index,
					      .end = state->input.end };

    if (range_is_empty (input))
    {
	if (state->input_is_finished)
	{
	    write_literals(output, state);
	}
	
	return false;
    }

    if (!state->input_is_finished && (size_t)range_count (input) < state->window_size)
    {
	return false;
    }
    
    assert (range_count (input) > 0);
    
    dzip_match match;
    size_t size;
    
    size_t metadata_threshold_size = 5;
    
    assert (state->input.begin + state->input_index < state->input.end);

    if (metadata_threshold_size < (size = count_repeat (&input)))
    {
	//log_debug ("added repeat %d[%d]", *input.begin, size);
	
	write_literals(output, state);
	*buffer_push (*output) = DZIP_CMD_REPEAT;
	*buffer_push (*output) = *input.begin;
	vluint_write (output, size);

	state->input_index += size;
	state->literal_start = state->input_index;
	/*if (!(state->input.begin + state->input_index <= state->input.end))
	{
	    assert (size + input.begin <= input.end);
	    assert (state->input.end == input.end);
	    log_debug ("bad: %zu %zu", size, state->input.end - state->input.begin + state->input_index);
	    }*/
	assert (state->input.begin + state->input_index <= state->input.end);
    }
    else if (metadata_threshold_size < (size = match_offset (&match, state, &input)))
    {
	//log_debug ("added match (%06zu, %06zu, %03d)", match.distance, match.length, match.next_char);
	//log_debug ("match: %.*s", match.length, input.begin - match.distance);

	state->stats.match_count++;
	state->stats.match_size += match.length;
	state->stats.match_offset += match.distance;
	if (state->stats.longest_match < match.length)
	{
	    state->stats.longest_match = match.length;
	}
	
	write_literals(output, state);
	*buffer_push (*output) = DZIP_CMD_LZ77;
	vluint_write (output, match.distance);
	vluint_write (output, match.length);
	*buffer_push (*output) = match.next_char;

	assert (size == match.length);
	state->input_index += size;
	state->literal_start = state->input_index;
	assert (state->input.begin + state->input_index <= state->input.end);
    }
    else
    {
	size_t skip_size = 1 + state->skip_count;

	size = range_count (input);
	
	if (size > skip_size)
	{
	    size = skip_size;
	}
	
	state->input_index += size;
	assert (state->input.begin + state->input_index <= state->input.end);
	//state->input_index += 1;
    }
    
    //print_int ("mb to go", range_count(input) / (1024 * 1024));

    assert (state->input.begin + state->input_index <= state->input.end);
    assert ((long long int)state->input_index <= range_count(state->input));
    
    return state->input_index < (size_t)range_count(state->input);
}

bool dzip_inflate (buffer_unsigned_char * output, dzip_inflate_state * state, const range_const_unsigned_char * input)
{
    return false;
}

void dzip_deflate_print_stats (dzip_deflate_state * state)
{
    log_stderr ("Average literal size: %lf", (double)state->stats.literal_size / (double)state->stats.literal_count);
    log_stderr ("Average match size: %lf", (double)state->stats.match_size / (double)state->stats.match_count);
    log_stderr ("Average match offset: %lf", (double)state->stats.match_offset / (double)state->stats.match_count);
    log_stderr ("Average match offset as a percent of the window: %lf", ((double)state->stats.match_offset / (double)state->stats.match_count) / (double)state->window_size);
    log_stderr ("Longest match: %zu", state->stats.longest_match);
    log_stderr ("Match / Literal size ratio: %lf", (double)state->stats.match_size / (double)state->stats.literal_size);
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
