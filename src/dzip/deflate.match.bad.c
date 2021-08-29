#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "dzip.h"
#include "internal.h"
#include "../log/log.h"

typedef uint16_t dzip_window_point;
typedef uint16_t dzip_window_match_index;
typedef struct {
    dzip_window_point point;
    dzip_window_point length;
}
    dzip_match;

#define DZIP_ARG1_MAX (1 << 14)

#define count_array(array) (sizeof(array) / sizeof((array)[0]))

typedef struct {
    char begin[65536];
    dzip_window_point match[33863*33];
    dzip_window_point point;
}
    dzip_window;

struct dzip_deflate_state {
    dzip_window window;
    buffer_char literal;
};

static void write_command (buffer_char * output, unsigned char command, unsigned short arg)
{
    assert (arg < DZIP_ARG1_MAX);
    assert (command < 4);

    if (arg < 64)
    {
	*buffer_push (*output) = 2 | command | (arg << 2);
    }
    else
    {
	uint16_t write = command;
	write |= arg << 2;
	
	buffer_append_n(*output, (const char*)&write, sizeof(write));
    }
}

static void write_point (buffer_char * restrict output, dzip_window_point n)
{
    buffer_append_n (*output, (const char*)&n, sizeof(n));
}

static void write_match (buffer_char * restrict output, dzip_match * restrict match)
{
    //log_debug ("writing match %zu %zu", match->length, match->point);
    write_command (output, DZIP_CMD_MATCH, match->length);
    write_point(output, match->point);
}

static void write_literal (buffer_char * restrict output, buffer_char * restrict literal)
{
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

inline static void add_window_char (dzip_window * restrict window, char c)
{
    window->begin[window->point] = c;
    if (sizeof(dzip_window_match_index) < window->point)
    {
	dzip_window_point match_point = window->point - sizeof(dzip_window_match_index);
	dzip_window_match_index match_index = *(dzip_window_match_index*) (window->begin + match_point);

	window->match[match_index % count_array (window->match)] = match_point;
    }
    window->point = (window->point + 1) % count_array (window->begin);
}

inline static void find_match (dzip_match * restrict match, dzip_deflate_state * state, const range_const_char * restrict input)
{
    dzip_size input_size = range_count (*input);

    if (input_size > DZIP_ARG1_MAX - 1)
    {
	input_size = DZIP_ARG1_MAX - 1;
    }

    dzip_window_match_index match_index = *(dzip_window_match_index*) input;

    setup_match (match, &state->window, state->window.match[match_index % count_array(state->window.match)], input->begin, input_size);

    log_debug ("length: %zu", match->length);
}

keyargs_define(dzip_deflate)
{
    dzip_match match;
    range_const_char input = *args.input;

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
}

keyargs_define(dzip_deflate_state_new)
{
    return calloc (1, sizeof (dzip_deflate_state));
}
