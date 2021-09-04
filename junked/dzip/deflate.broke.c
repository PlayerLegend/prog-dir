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

typedef uint64_t dzip_index;

typedef struct {
    dzip_window_point distance;
    dzip_window_point length;
}
    dzip_match;

typedef struct {
    dzip_size begin;
    dzip_window_point length;
}
    dzip_recent;

typedef struct {
    dzip_size bytes_read;
    dzip_window window;
    dzip_recent recent[6857];
}
    dzip_deflate_past;

struct dzip_deflate_state {
    dzip_deflate_past past;
    buffer_char literal;
    dzip_match match;
};

static void write_command (buffer_char * output, unsigned char command, unsigned short arg)
{
    assert (arg < DZIP_ARG1_EXTEND_MAX);
    assert (command < DZIP_COMMAND_MAX);

    unsigned char header = (command & DZIP_COMMAND_MASK) | ( (arg % DZIP_ARG1_COMPACT_MAX) << DZIP_META_BITS );
    
    if (arg < DZIP_ARG1_COMPACT_MAX)
    {
	*buffer_push (*output) = header;
    }
    else
    {
	*buffer_push (*output) = header | DZIP_ARG1_EXTEND_BIT;
	*buffer_push (*output) = arg / DZIP_ARG1_COMPACT_MAX;
    }
}

static void write_point (buffer_char * restrict output, dzip_window_point n)
{
    buffer_append_n (*output, (const char*)&n, sizeof(n));
}

inline static void write_literal (buffer_char * output, dzip_deflate_state * state)
{
    assert ((size_t)range_count(state->literal) >= state->match.length);
    dzip_size size = range_count (state->literal) - state->match.length;
    assert (size < DZIP_ARG1_EXTEND_MASK);

    write_command (output, DZIP_CMD_LITERAL, size);

    buffer_append_n(*output, state->literal.begin, size);
    buffer_rewrite (state->literal);
}

inline static void write_match (buffer_char * output, dzip_deflate_state * state)
{
    write_command (output, DZIP_CMD_MATCH, state->match.length);
    write_point(output, state->match.distance);
    state->match.length = 0;
}

inline static void write_next (buffer_char * output, dzip_deflate_state * state)
{
    if (4 < state->match.length)
    {
	write_literal (output, state);
	write_match (output, state);
    }
    else
    {
	state->match.length = 0;
	write_literal (output, state);
    }
}

inline static void setup_match (dzip_deflate_state * restrict state, dzip_recent * restrict recent, char byte)
{
    state->match.distance = (state->past.bytes_read - recent->begin) % count_array(state->past.window);

    if (reference_window_byte (state->past.window, state->match.distance) == byte)
    {
	state->match.length = 1;
    }
    else
    {
	state->match.length = 0;
    }
}

inline static void add_state_byte (buffer_char * output, dzip_deflate_state * state, const char * restrict input)
{
    union {
	dzip_index index;
	unsigned char byte;
    }
    register future = { .index = *(dzip_index*) input };

    dzip_recent * restrict recent = &reference_past_point(state->past, future.index);

    dzip_window_point literal_length = range_count(state->literal);

    if ( (state->match.length && future.byte != reference_window_byte (state->past.window, state->past.bytes_read - state->match.distance))
	 || state->match.length == DZIP_ARG1_EXTEND_MASK
	 || literal_length == DZIP_ARG1_EXTEND_MASK)
    {
	write_next (output, state);
	setup_match (state, recent, future.byte);
    }
    else if (!state->match.length)
    {
	setup_match (state, recent, future.byte);
    }
    else
    {
	state->match.length++;
    }
    
    *recent = (dzip_recent){ .begin = state->past.bytes_read };
    *buffer_push (state->literal) = future.byte;
    reference_window_byte(state->past.window, state->past.bytes_read) = future.byte;
    state->past.bytes_read++;
}

inline static void load_literal (dzip_deflate_state * state, const char * restrict input, dzip_size size)
{
    buffer_append_n(state->literal, input, size);
    for (dzip_size i = 0; i < size; i++)
    {
	reference_window_byte(state->past.window, state->past.bytes_read) = input[i];
	state->past.bytes_read++;	
    }
}

keyargs_define(dzip_deflate)
{
    range_const_char input = *args.input;
    
    if (!args.state->past.bytes_read)
    {
	buffer_append_n (*args.output, "deezwhat", 8);
    }

    if ((size_t)range_count (input) <= 2 * sizeof(dzip_index))
    {
	load_literal (args.state, input.begin, range_count(input));
	write_literal (args.output, args.state);
	return;
    }
    
    input.end -= sizeof(dzip_index);
    
    while (input.begin < input.end)
    {
	add_state_byte (args.output, args.state, input.begin);
	input.begin++;
    }

    write_next (args.output, args.state);

    load_literal (args.state, input.end, sizeof(dzip_index));
    write_literal (args.output, args.state);
    
#ifdef DZIP_RECORD_STATS
    stat_bytes_in += range_count (*args.input);
    stat_bytes_out += range_count (*args.output);
#endif
}

keyargs_define(dzip_deflate_state_new)
{
    return calloc (1, sizeof (dzip_deflate_state));
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
