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
    stat_bytes_in,
    stat_bytes_out;
#endif

typedef struct {
    dzip_window_point point;
    dzip_window_point length;
}
    dzip_match;

#define count_array(array) (sizeof(array) / sizeof((array)[0]))

struct dzip_deflate_state {
    dzip_window window;
    dzip_window_point match_recent[65536];
    dzip_window_point match_best[33863];
    buffer_char literal;
    bool header_is_written;
};

static void write_command (buffer_char * output, unsigned char command, unsigned short arg)
{
    assert (arg < DZIP_ARG1_MAX);
    assert (command < 4);

    if (arg < 32)
    {
	*buffer_push (*output) = command |DZIP_ARG1_EXTEND_BIT | (arg << (16 - DZIP_ARG1_BITS));
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
    
    //log_debug ("writing match %zu %zu", match->length, match->point);
    write_command (output, DZIP_CMD_MATCH, match->length);
    write_point(output, match->point);
}

static void write_literal (buffer_char * restrict output, buffer_char * restrict literal)
{
#ifdef DZIP_RECORD_STATS
    stat_literal_count++;
    stat_literal_length += range_count (*literal);
#endif
    
    //log_debug ("writing literal %zu", range_count (*literal));
    write_command (output, DZIP_CMD_LITERAL, range_count(*literal));
    buffer_append (*output, *literal);
    buffer_rewrite (*literal);
}

inline static void setup_match (dzip_match * restrict match, dzip_window * restrict window, dzip_window_point point, const char * restrict input_begin, dzip_size input_size)
{
    match->point = point;

    match->length = 0;

    while (match->length < input_size && input_begin[match->length] == window->begin[(point + match->length) % sizeof (window->begin)])
    {
	match->length++;
    }
}

inline static void find_match (dzip_match * restrict match, dzip_deflate_state * state, const range_const_char * restrict input)
{
    dzip_size input_size = range_count (*input);

    if (input_size > DZIP_ARG1_MAX - 1)
    {
	input_size = DZIP_ARG1_MAX - 1;
    }
    
    dzip_window_point * best_point_p = state->match_best + (*(uint64_t*) input->begin) % count_array (state->match_best);
    dzip_match best_match;
    setup_match (&best_match, &state->window, *best_point_p, input->begin, input_size);
    
    if (best_match.length >= 8)
    {
	goto use_best;
    }
    
    dzip_match recent_match;
    setup_match (&recent_match, &state->window, state->match_recent[(*(uint16_t*) input->begin) % count_array (state->match_recent)], input->begin, input_size);

    if (best_match.length > recent_match.length)
    {
	goto use_best;
    }
    else
    {
	goto use_recent;
    }

use_best:
    //log_debug ("best %zu(%zu) > %zu(%zu)", best_match.length, best_match.point, recent_match.length, recent_match.point);
    *match = best_match;
    return;

use_recent:
    *match = recent_match;
    *best_point_p = recent_match.point;
    return;
}

inline static void add_window_char (dzip_window * restrict window, char c)
{
    window->begin[window->point] = c;
    window->point = (window->point + 1) % sizeof (window->begin);
}

inline static void add_recent_char (dzip_deflate_state * state, const char * restrict input_begin)
{
    state->match_recent[*(uint16_t*) input_begin % count_array(state->match_recent)] = state->window.point;
}

keyargs_define(dzip_deflate)
{
    dzip_match match = {0};
    range_const_char input = *args.input;

    if (!args.state->header_is_written)
    {
	buffer_append_n (*args.output, "deezwhat", 8);
	args.state->header_is_written = true;
    }

    input.end -= 8;

    while (input.begin < input.end)
    {
	find_match (&match, args.state, &input);
	if (match.length > 4)
	{
	    if (range_count (args.state->literal))
	    {
		write_literal(args.output, &args.state->literal);
	    }
	    
	    write_match(args.output, &match);
	    
	    for (int i = 0; i < match.length; i++)
	    {
		add_window_char (&args.state->window, *(input.begin++));
	    }
	}
	else
	{
	    *buffer_push (args.state->literal) = *input.begin;
	    add_recent_char (args.state, input.begin);
	    
	    if (range_count (args.state->literal) == DZIP_ARG1_MAX - 1)
	    {
		write_literal(args.output, &args.state->literal);
	    }
	    
	    add_window_char (&args.state->window, *(input.begin++));
	}
    }

    write_literal (args.output, &args.state->literal);
    
    input.begin = input.end;
    input.end = args.input->end;

    buffer_append (args.state->literal, input);
    write_literal (args.output, &args.state->literal);

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
