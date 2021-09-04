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
    dzip_window_point point;
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
    dzip_size literal_begin;
    dzip_size match_to_begin;
    dzip_size match_from_begin;
};

static void write_command (buffer_char * output, unsigned char command, unsigned short arg)
{
    assert (arg < DZIP_ARG1_EXTEND_MAX);
    assert (command < DZIP_COMMAND_MAX);

    //log_debug ("Write command: %zu %zu", command, arg);

    unsigned char header = (command & DZIP_COMMAND_MASK) | ( (arg % DZIP_ARG1_COMPACT_MAX) << DZIP_META_BITS );
    
    if (arg < DZIP_ARG1_COMPACT_MAX)
    {
	//log_debug ("Write command compact(%zu): %zu %zu [%zu]", header, command, arg, range_count(*output));
	//log_debug ("Mask %zu and %zu", arg, DZIP_ARG1_COMPACT_MASK);
	*buffer_push (*output) = header;
    }
    else
    {
	//log_debug ("Write command extended: %zu %zu [%zu]", command, arg, range_count(*output));
	
	*buffer_push (*output) = header | DZIP_ARG1_EXTEND_BIT;
	*buffer_push (*output) = arg / DZIP_ARG1_COMPACT_MAX;
	//log_debug ("wrote extend %zu", arg / DZIP_ARG1_COMPACT_MAX);
    }
}

static void write_point (buffer_char * restrict output, dzip_window_point n)
{
    buffer_append_n (*output, (const char*)&n, sizeof(n));
}

inline static void finalize_literal (buffer_char * output, dzip_deflate_state * state)
{
    assert (state->match_to_begin >= state->literal_begin);

    dzip_window_point literal_length = state->match_to_begin - state->literal_begin;

    if (!literal_length)
    {
	return;
    }

#ifdef DZIP_RECORD_STATS
    stat_literal_count++;
    stat_literal_length += literal_length;
#endif

    //log_debug ("wrote literal: %zu", literal_length);
    
    write_command (output, DZIP_CMD_LITERAL, literal_length);
    
    while (state->literal_begin < state->match_to_begin)
    {
	*buffer_push(*output) = reference_window_byte (state->past.window, state->literal_begin);
	//log_debug ("write literal[%zu]: %u", state->literal_begin, output->end[-1]);
	state->literal_begin++;
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

    dzip_window_point match_length = state->past.bytes_read - state->match_to_begin;
    assert (state->past.bytes_read >= state->match_to_begin);
    assert (state->match_to_begin >= state->match_from_begin);

    dzip_window_point literal_length = state->past.bytes_read - state->literal_begin;

    if (future.byte != reference_window_byte (state->past.window, state->match_from_begin + match_length) || match_length == DZIP_ARG1_EXTEND_MASK || literal_length == DZIP_ARG1_EXTEND_MASK)
    {
	if (4 < match_length)
	{
	    dzip_window_point match_distance = (state->match_to_begin - state->match_from_begin) % count_array (state->past.window);

	    finalize_literal(output, state);

	    write_command (output, DZIP_CMD_MATCH, match_length);
	    write_point (output, match_distance);

#ifdef DZIP_RECORD_STATS
	    stat_match_count++;
	    stat_match_length += match_length;
	    stat_match_distance += match_distance;
#endif

	    //log_debug ("wrote match: %zu %zu", match_length, match_distance);
	    assert (state->match_to_begin + match_length == state->past.bytes_read);
	    
	    state->literal_begin = state->past.bytes_read;
	}
	else if (literal_length == DZIP_ARG1_EXTEND_MASK)
	{
	    state->match_to_begin = state->past.bytes_read;
	    finalize_literal(output, state);
	}
	
	state->match_to_begin = state->past.bytes_read;
	state->match_from_begin = recent->begin;
		
	if (future.byte != reference_window_byte (state->past.window, state->match_from_begin))
	{
	    state->match_to_begin++;
	}
    }
    //if (state->past.bytes_read % 2 == 0)
	*recent = (dzip_recent){ .begin = state->past.bytes_read };
    reference_window_byte(state->past.window, state->past.bytes_read) = future.byte;
    state->past.bytes_read++;
}

inline static void write_literal (buffer_char * output, dzip_deflate_state * state, dzip_size size, const char * begin)
{
    state->match_to_begin = state->past.bytes_read;
    finalize_literal(output, state);
    for (dzip_size i = 0; i < size; i++)
    {
	reference_window_byte(state->past.window, state->past.bytes_read) = begin[i];
	state->past.bytes_read++;
    }
    state->match_to_begin = state->past.bytes_read;
    finalize_literal(output, state);
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
	write_literal (args.output, args.state, range_count (input), input.begin);
	return;
    }
    
    input.end -= sizeof(dzip_index);
    
    while (input.begin < input.end)
    {
	add_state_byte (args.output, args.state, input.begin);
	input.begin++;
    }
    
    write_literal (args.output, args.state, sizeof(dzip_index), input.end);
    
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
