#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "dzip.h"
#include "internal.h"
#include "../log/log.h"
#include "../vluint/vluint.h"

//#ifndef NDEBUG
#define DZIP_RECORD_STATS
//#endif

#ifdef DZIP_RECORD_STATS
dzip_size stat_literal_count,
    stat_literal_length,
    stat_match_count,
    stat_match_length,
    stat_match_distance,
    stat_bytes_in,
    stat_bytes_out;
#endif

typedef uint32_t dzip_index;

typedef struct {
    dzip_size distance;
    dzip_size length;
}
    dzip_match;

struct dzip_deflate_state {
    dzip_size window_progress;
    dzip_size literal_begin;
    dzip_match match;
    dzip_size recent[16963];
};

#define reference_array(array, index)			\
    ( (array)[(index) % count_array(array)] )

static void write_command (buffer_unsigned_char * output, unsigned char command, dzip_size arg)
{
    assert (command < DZIP_COMMAND_MAX);
    assert (arg > 0);
    
    unsigned char header = (command & DZIP_COMMAND_MASK) | ( (arg % DZIP_ARG1_COMPACT_MAX) << DZIP_META_BITS );
    
    if (arg < DZIP_ARG1_COMPACT_MAX)
    {
	*buffer_push (*output) = header;
    }
    else
    {
	*buffer_push (*output) = header | DZIP_ARG1_EXTEND_BIT;
	vluint_write (output, arg / DZIP_ARG1_COMPACT_MAX);
    }
}

static void write_literal (buffer_unsigned_char * restrict output, dzip_deflate_state * state, const range_const_unsigned_char * window)
{
    assert (state->literal_begin <= state->window_progress);
    
    if (state->literal_begin < state->window_progress)
    {
	assert (window->begin + state->window_progress <= window->end);
	size_t length = state->window_progress - state->literal_begin;

	write_command (output, DZIP_CMD_LITERAL, length);
	const unsigned char * begin = window->begin + state->literal_begin;
	assert (begin + length <= window->end);
	buffer_append_n (*output, begin, length);
	state->literal_begin = state->window_progress;

#ifdef DZIP_RECORD_STATS
	stat_literal_count++;
	stat_literal_length += length;
#endif
    }
}

static void write_match (buffer_unsigned_char * restrict output, dzip_deflate_state * state)
{
    assert (state->match.length);
    
    write_command (output, DZIP_CMD_MATCH, state->match.length);
    vluint_write (output, state->match.distance);
    state->window_progress += state->match.length;
    state->literal_begin = state->window_progress;

    assert (state->match.distance > 0);
    
#ifdef DZIP_RECORD_STATS
    stat_match_count++;
    stat_match_length += state->match.length;
    stat_match_distance += state->match.distance;
#endif
}

static void check_match (dzip_match * match, range_const_unsigned_char * scan)
{
    const unsigned char * i;

    for_range (i, *scan)
    {
	if (i[0] != i[-(size_t)match->distance])
	{
	    break;
	}
    }

    match->length = i - scan->begin;
}

static void setup_match (dzip_deflate_state * state, const range_const_unsigned_char * window)
{
    range_const_unsigned_char scan = { .begin = window->begin + state->window_progress, .end = window->end };
    
    dzip_index * index = (void*) scan.begin;

    assert ((int)sizeof(*index) <= range_count (scan));
    
    dzip_size * recent = &reference_array(state->recent, *index);

    if (*recent >= state->window_progress)
    {
	*recent = state->window_progress;
	state->match.length = 0;
	return;
    }
    
    state->match.distance = state->window_progress - *recent;

    assert (state->match.distance > 0);

    check_match (&state->match, &scan);

    *recent = state->window_progress;
}

inline static void deflate_chunk (buffer_unsigned_char * restrict output, dzip_deflate_state * restrict state, const range_const_unsigned_char * input)
{
    dzip_size output_start = range_count(*output);
    dzip_size window_size = range_count (*input);

    buffer_resize (*output, output_start + sizeof(dzip_header));
    	
    state->window_progress = 8;
    state->literal_begin = 0;

    if (window_size > sizeof(dzip_index))
    {
	dzip_size max = window_size - sizeof(dzip_index);
	while (state->window_progress < max)
	{
	    assert (input->begin + state->window_progress <= input->end - sizeof(dzip_index));

	    setup_match (state, input);

	    if (4 < state->match.length)
	    {
		write_literal (output, state, input);
		write_match (output, state);
	    }
	    else
	    {
		state->window_progress++;
	    }
	}
    }
    
    state->window_progress = window_size;
    write_literal(output, state, input);

    *(dzip_header*) (output->begin + output_start) = (dzip_header)
	{
	    .magic = DZIP_MAGIC_INITIALIZER,
	    .version = DZIP_VERSION,
	    .chunk_size = range_count(*output) - output_start,
	};
}

keyargs_define(dzip_deflate)
{
    range_const_unsigned_char chunk = *args.input;
    dzip_size max_chunk_size = 1e6;
    
    while (chunk.begin < args.input->end)
    {
	chunk.end = chunk.begin + max_chunk_size;

	if (chunk.end > args.input->end)
	{
	    chunk.end = args.input->end;
	}
	
	deflate_chunk (args.output, args.state, &chunk);

	chunk.begin = chunk.end;
    }
    
#ifdef DZIP_RECORD_STATS
    if (args.input)
    {
	stat_bytes_in += range_count (*args.input);
    }
    
    stat_bytes_out += range_count (*args.output);
#endif
}

keyargs_define(dzip_deflate_state_new)
{
    return calloc (1, sizeof (dzip_deflate_state));
}

void dzip_deflate_state_free(dzip_deflate_state * state)
{
    free (state);
}

void dzip_print_stats()
{
#ifdef DZIP_RECORD_STATS
    log_stderr ("Output bytes / Input bytes = %zu / %zu = %lf", stat_bytes_out, stat_bytes_in, (double) stat_bytes_out / (double) stat_bytes_in);
    log_stderr ("Literal weight in input = %zu / %zu = %lf", stat_literal_length, stat_bytes_in, (double) stat_literal_length / (double) stat_bytes_in);
    log_stderr ("Match weight in input = %zu / %zu = %lf", stat_match_length, stat_bytes_in, (double) stat_match_length / (double) stat_bytes_in);
    log_stderr ("Average literal length = %zu / %zu = %lf", stat_literal_length, stat_literal_count, (double) stat_literal_length / (double) stat_literal_count);
    log_stderr ("Average match length = %zu / %zu = %lf", stat_match_length, stat_match_count, (double) stat_match_length / (double) stat_match_count);
    log_stderr ("Average match distance = %zu / %zu = %lf", stat_match_distance, stat_match_count, (double) stat_match_distance / (double) stat_match_count);
#endif
}
