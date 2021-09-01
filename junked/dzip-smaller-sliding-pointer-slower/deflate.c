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

typedef struct {
    dzip_window_point point;
    dzip_window_point length;
}
    dzip_match;

typedef union {
    //char bytes[8];
    struct { char byte_oldest; char _byte_reserve[6]; char byte_newest; };
    //struct { char byte_oldest; char _byte_reserve[2]; char byte_newest; };
    //struct { uint16_t i16_oldest; uint16_t _i16_reserve[2]; uint16_t i16_newest; };
    //struct { uint32_t i32_oldest; uint32_t i32_newest; };
    uint64_t full;
}
    dzip_sliding_index;

typedef struct {
    dzip_sliding_index index;
}
    dzip_deflate_future;

typedef struct {
    dzip_sliding_index index;
    dzip_size bytes_read;
    dzip_window window;
    dzip_size recent[33863 * 33];
}
    dzip_deflate_past;

struct dzip_deflate_state {
    dzip_deflate_past past;
    dzip_size literal_begin;
    dzip_size match_to_begin;
    dzip_size match_from_begin;
    uint64_t match_index;
    dzip_deflate_future future;
};

#define count_array(array) (sizeof(array) / sizeof((array)[0]))

#define reference_past_point(past, index)		\
    ( (past).recent[(index) % count_array((past).recent)] )

#define reference_window_byte(window, index)		\
    ( (window)[ (count_array(window) + index) % count_array(window) ] )

inline static void add_index_byte (dzip_sliding_index * restrict index, char byte)
{
    index->full >>= 8;
    index->byte_newest = byte;
}

inline static char add_future_byte (dzip_deflate_future * future, char byte)
{
    char retval = future->index.byte_oldest;
    add_index_byte (&future->index, byte);
    return retval;
}

inline static void add_past_byte (dzip_deflate_past * past, char c)
{
    reference_past_point(*past, past->index.full) = past->bytes_read - sizeof(past->index.full);
    reference_window_byte(past->window, past->bytes_read) = c;
    past->bytes_read++;
    add_index_byte(&past->index, c);
}

static void write_command (buffer_char * output, unsigned char command, unsigned short arg)
{
    assert (arg < DZIP_ARG1_MAX);
    assert (command < 4);

    if (arg < 32)
    {
	*buffer_push (*output) = command | DZIP_ARG1_EXTEND_BIT | (arg << (16 - DZIP_ARG1_BITS));
    }
    else
    {
	uint16_t write = command;
	write |= arg << (16 - DZIP_ARG1_BITS);
	
	buffer_append_n(*output, (const char*)&write, sizeof(write));
    }
}

static void write_point (buffer_char * restrict output, dzip_window_point n)
{
    buffer_append_n (*output, (const char*)&n, sizeof(n));
}

inline static void finalize_literal (buffer_char * output, dzip_deflate_state * state, dzip_size max)
{
    assert (max >= state->literal_begin);

    dzip_window_point literal_length = max - state->literal_begin;

    if (!literal_length)
    {
	return;
    }

#ifdef DZIP_RECORD_STATS
    stat_literal_count++;
    stat_literal_length += literal_length;
#endif

    //log_debug ("Literal: %zu", literal_length);
    
    write_command (output, DZIP_CMD_LITERAL, literal_length);
    
    while (state->literal_begin < max)
    {
	*buffer_push(*output) = reference_window_byte (state->past.window, state->literal_begin);
	state->literal_begin++;
    }
}

inline static void add_state_byte (buffer_char * output, dzip_deflate_state * state, char future)
{
    assert (state->past.bytes_read >= sizeof(state->future.index));

    dzip_window_point match_length = state->past.bytes_read - state->match_to_begin;
    assert (state->past.bytes_read >= state->match_to_begin);
    assert (state->match_to_begin >= state->match_from_begin);

    dzip_window_point literal_length = state->past.bytes_read - state->literal_begin;

    if (state->future.index.byte_oldest != reference_window_byte (state->past.window, state->match_from_begin + match_length) || match_length == DZIP_ARG1_MAX - 1 || literal_length == DZIP_ARG1_MAX - 1)
    {
	if (4 < match_length)
	{
	    dzip_window_point match_distance = (state->match_to_begin - state->match_from_begin) % count_array (state->past.window);

	    //log_debug ("match: %zu %zu", match_length, match_distance);
	    
	    finalize_literal(output, state, state->match_to_begin);

	    //assert (match_distance < DZIP_ARG1_MAX);
	    
	    write_command (output, DZIP_CMD_MATCH, match_length);
	    write_point (output, match_distance);

#ifdef DZIP_RECORD_STATS
	    stat_match_count++;
	    stat_match_length += match_length;
	    stat_match_distance += match_distance;
#endif
	    
	    assert (state->match_to_begin + match_length == state->past.bytes_read);
	    
	    state->literal_begin = state->past.bytes_read;
	}
	else if (literal_length == DZIP_ARG1_MAX - 1)
	{
	    finalize_literal(output, state, state->past.bytes_read);
	}
        
	state->match_to_begin = state->past.bytes_read;
	state->match_from_begin = reference_past_point(state->past, state->future.index.full);
    }

    add_past_byte(&state->past, state->future.index.byte_oldest);
    add_future_byte (&state->future, future);
}

keyargs_define(dzip_deflate)
{
    range_const_char input = *args.input;

    while (args.state->past.bytes_read < sizeof(dzip_sliding_index) && input.begin < input.end)
    {
	if (!args.state->past.bytes_read)
	{
	    buffer_append_n (*args.output, "deezwhat", 8);
	}
	
	add_past_byte(&args.state->past, args.state->future.index.byte_oldest);
	add_future_byte (&args.state->future, *input.begin);
	input.begin++;
	
    }
    
    while (input.begin < input.end)
    {
	add_state_byte (args.output, args.state, *input.begin);
	input.begin++;
    }

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
