#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
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
    stat_bytes_in,
    stat_bytes_out;
#endif

typedef struct {
    dzip_window_point point;
    dzip_window_point length;
    bool ended;
}
    dzip_match;

#define count_array(array) (sizeof(array) / sizeof((array)[0]))

typedef union {
    char bytes[8];
    struct { char byte_oldest; char _byte_reserve[6]; char byte_newest; };
    uint16_t i16;
    uint32_t i32;
    uint64_t i64;
}
    dzip_sliding_index;

typedef struct {
    dzip_window window;
    dzip_window_point point_table_recent[65536];
    dzip_window_point point_table_best[33863];
    dzip_sliding_index index;
    dzip_size bytes_read;
}
    dzip_stream;

struct dzip_deflate_state {
    dzip_stream stream;
    struct {
	buffer_char literal;
	dzip_match match_best;
	dzip_match match_recent;
	uint64_t match_recent_index;
    }
	possible;
};

#define get_point_table_p(table, index)		\
    ( (table) + (index) % count_array(table) )

#define get_window_byte(window, index)		\
    ( (window).begin[ ( count_array((window).begin) + (index) ) % count_array( (window).begin ) ] )

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

static void write_match (buffer_char * restrict output, dzip_match * restrict match)
{
#ifdef DZIP_RECORD_STATS
    stat_match_count++;
    stat_match_length += match->length;
#endif

    assert (match->length);
    //log_debug ("writing match %zu %zu", match->length, match->point);
    write_command (output, DZIP_CMD_MATCH, match->length);
    write_point(output, match->point);
}

static void write_literal (buffer_char * restrict output, dzip_size length, const char * restrict begin)
{
    if (!length)
    {
	return;
    }
    
    assert (length < DZIP_ARG1_MAX);
    
#ifdef DZIP_RECORD_STATS
    stat_literal_count++;
    stat_literal_length += length;
#endif
    
    //log_debug ("writing literal %zu", length);
    write_command (output, DZIP_CMD_LITERAL, length);
    buffer_append_n (*output, begin, length);
}

inline static void add_window_char (dzip_window * restrict window, char c)
{
    window->begin[window->point] = c;
    window->point = (window->point + 1) % sizeof (window->begin);
}

inline static void add_index_byte (dzip_sliding_index * restrict index, char byte)
{
    index->i64 >>= 8;
    index->byte_newest = byte;
}

inline static void start_match (dzip_match * restrict match, dzip_window * restrict window, dzip_window_point point, char c)
{
    assert (point < count_array (window->begin));
    if (point != window->point)
    {
	//log_debug ("starting match at %zu", point);
	*match = (dzip_match){ .point = point };
    }
    else
    {
	*match = (dzip_match){ .ended = true };
    }
}

inline static void update_literal (buffer_char * restrict literal, dzip_window * restrict window, char c)
{
    *buffer_push(*literal) = c;
}

inline static void update_match (dzip_match * restrict match, dzip_window * restrict window, char c)
{
    if (!match->ended)
    {
	if (get_window_byte (*window, match->point + match->length) == c)
	{
	    match->length++;
	}
	else
	{
	    // log_debug ("Ended match at %zu", match->length);
	    match->ended = true;
	}
    }
}

inline static void check_match (dzip_window * window, dzip_match * match)
{
    assert ( (match->point + match->length) % count_array(window->begin) != window->point);
    
    for (int i = 0; i < match->length; i++)
    {
	assert (get_window_byte(*window, match->point + i) == get_window_byte(*window, window->point - match->length + i));
    }
}

inline static void write_match_or_literal (buffer_char * restrict output, dzip_match * restrict match, buffer_char * restrict literal)
{
    if (3 < match->length)
    {
	assert (range_count (*literal) >= match->length);
	
	if (range_count (*literal) > match->length)
	{
	    write_literal (output, range_count (*literal) - match->length, literal->begin);
	}

	write_match(output, match);
	buffer_rewrite (*literal);
    }
    else if (range_count (*literal) == DZIP_ARG1_MAX - 1)
    {
	write_literal(output, range_count (*literal), literal->begin);
	buffer_rewrite (*literal);
    }
    assert (range_count(*literal) < DZIP_ARG1_MAX);
}
	
inline static char update_stream (dzip_stream * stream, char c)
{
    char oldest = stream->index.byte_oldest;
    //update_stream_recent_point_table (stream);
    add_window_char (&stream->window, stream->index.byte_oldest);
    add_index_byte(&stream->index, c);
    stream->bytes_read++;
    return oldest;
}

dzip_sliding_index test_index;

inline static void update_state (buffer_char * restrict output, dzip_deflate_state * restrict state, char add)
{
    update_match(&state->possible.match_best, &state->stream.window, state->stream.index.byte_oldest);
    update_match(&state->possible.match_recent, &state->stream.window, state->stream.index.byte_oldest);
    update_literal (&state->possible.literal, &state->stream.window, state->stream.index.byte_oldest);  
    if ( (state->possible.match_recent.ended && state->possible.match_best.ended) || range_count (state->possible.literal) == DZIP_ARG1_MAX - 1)
    {
	if (state->possible.match_best.length > state->possible.match_recent.length)
	{
	    write_match_or_literal (output, &state->possible.match_best, &state->possible.literal);
	}
	else
	{
	    if (state->possible.match_recent.length >= 6)
	    {
		*get_point_table_p(state->stream.point_table_best, state->possible.match_recent_index) = state->possible.match_recent.point % count_array(state->stream.window.begin);
	    }
	    
	    write_match_or_literal (output, &state->possible.match_recent, &state->possible.literal);
	}
        
	start_match (&state->possible.match_recent,
		     &state->stream.window,
		     *get_point_table_p(state->stream.point_table_recent, state->stream.index.i16),
		     state->stream.index.byte_oldest);
	
	start_match (&state->possible.match_best,
		     &state->stream.window,
		     *get_point_table_p(state->stream.point_table_best, state->stream.index.i64),
		     state->stream.index.byte_oldest);

	update_match(&state->possible.match_best, &state->stream.window, state->stream.index.byte_oldest);
	update_match(&state->possible.match_recent, &state->stream.window, state->stream.index.byte_oldest);
	
	state->possible.match_recent_index = state->stream.index.i64;
    }
    
    //update_match(&state->possible.match_best, &state->stream.window, state->stream.index.byte_oldest);
    //update_match(&state->possible.match_recent, &state->stream.window, state->stream.index.byte_oldest);
    *get_point_table_p(state->stream.point_table_recent, state->stream.index.i16) = state->stream.window.point;
    add_window_char(&state->stream.window, state->stream.index.byte_oldest);
    add_index_byte(&state->stream.index, add);

    check_match (&state->stream.window, &state->possible.match_best);
    check_match (&state->stream.window, &state->possible.match_recent);
}

keyargs_define(dzip_deflate)
{
    if (!args.input)
    {
	write_literal (args.output, range_count (args.state->possible.literal), args.state->possible.literal.begin);
	buffer_rewrite (args.state->possible.literal);
	return;
    }
    
    range_const_char input = *args.input;

    while (args.state->stream.bytes_read < sizeof (args.state->stream.index) && input.begin < input.end)
    {
	if (!args.state->stream.bytes_read)
	{
	    buffer_append_n (*args.output, "deezwhat", 8);
	}
	
	update_stream (&args.state->stream, *input.begin);

	input.begin++;
    }

    while (input.begin < input.end)
    {
	update_state (args.output, args.state, *input.begin);
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
#endif
}
