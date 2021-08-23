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
#include "../sliding-window/sliding-window.h"

typedef enum {
    DZIP_CMD_LZ77,
    DZIP_CMD_LITERAL,
    DZIP_CMD_REPEAT,
}
    dzip_command;

struct dzip_deflate_state {
    sliding_window window;
    buffer_unsigned_char input;
    buffer_unsigned_char literals;
    size_t input_shift;

    bool input_finished;
};

typedef struct {
    size_t distance;
    size_t length;
    char next_char;
}
    dzip_match;

keyargs_define(dzip_deflate_new)
{
    dzip_deflate_state * retval = calloc (1, sizeof(*retval));
    size_t window_size = args.window_size ? args.window_size : DZIP_DEFAULT_WINDOW_SIZE;

    sliding_window_init(&retval->window, window_size);
    
    log_debug ("window size: %zu", window_size);
    
    return retval;
}

dzip_inflate_state * dzip_inflate_new()
{
    return NULL;
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

void dump_range(const range_const_unsigned_char * range, size_t count)
{
    printf("range:\n");
    const unsigned char * i;
    for_range (i, *range)
    {
	if (range_index(i, *range) > count)
	{
	    break;
	}
	
	printf("\t%d\n", *i);
    }
    printf("\n");
}

static void print_int(const char * message, int n)
{
    static int number;

    if (number != n)
    {
	number = n;
	printf("%s: %d\n", message,  n);
    }
}

void dzip_deflate_add (dzip_deflate_state * state, const range_const_unsigned_char * input)
{
    if (!input || range_is_empty(*input))
    {
	state->input_finished = true;
    }
    else
    {
	buffer_append(state->input, *input);
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
/*
static size_t match_length (unsigned char * a, size_t a_size, unsigned char * b, size_t b_size)
{
    size_t min = a < b ? a : b;

    size_t i;

    for (i = 0; i < min; i++)
    {
	if (a[i] != b[i])
	{
	    break;
	}
    }

    return i;
}
*/
static size_t match_window (dzip_match * match, sliding_window * window, const range_const_unsigned_char * shift_input)
{
    size_t test_len;

    *match = (dzip_match){.next_char = *shift_input->begin};
    size_t max_match_length = range_count(*shift_input) - 1;
    
    for (size_t i = 0; i < window->size && match->length < max_match_length; i++)
    {
	//log_debug("%zu", i);
	test_len = sliding_window_match_length(window, window->size - 1 - i, shift_input);

	if (test_len > match->length)
	{
	    match->length = test_len;
	    match->distance = i;

	    if (match->length > max_match_length)
	    {
		match->length = max_match_length;
	    }
	    
	    match->next_char = shift_input->begin[match->length];
	}
	
	/*if (match->length > 10)
	{
	    break;
	    }*/
    }

    //log_debug ("(%06zu, %06zu, %03d) %zu", match->distance, match->length, match->next_char, max_match_length);
    return match->length;
}

void write_literals (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    if (!range_is_empty(state->literals))
    {
	//log_debug ("added literal %d", range_count(state->literals));
	*buffer_push (*output) = DZIP_CMD_LITERAL;
	vluint_write (output, range_count(state->literals));
	buffer_append(*output, state->literals);
	buffer_rewrite (state->literals);
    }
}

bool dzip_deflate_update (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    if (range_is_empty (state->input))
    {
	write_literals(output, state);
	return false;
    }
    
    if (state->input_shift > 1e5)
    {
	buffer_downshift(&state->input, state->input_shift);
	state->input_shift = 0;
    }
    
    const range_const_unsigned_char shift_input = { .begin = state->input.begin + state->input_shift,
						    .end = state->input.end };

    dzip_match match;
    size_t size;

    size_t literals_size = range_count(state->literals);

    size_t metadata_threshold_size = 10;

    bool preserve_literals = literals_size > 0 && literals_size < metadata_threshold_size;
    //preserve_literals = true;

    if (!preserve_literals && metadata_threshold_size < (size = count_repeat (&shift_input)))
    {
	//log_debug ("added repeat %d[%d]", *shift_input.begin, size);
	
	write_literals(output, state);
	*buffer_push (*output) = DZIP_CMD_REPEAT;
	*buffer_push (*output) = *shift_input.begin;
	vluint_write (output, size);
    }
    else if (!preserve_literals && metadata_threshold_size < (size = match_window (&match, &state->window, &shift_input)))
    {
	//log_debug ("added match (%06zu, %06zu, %03d)", match.distance, match.length, match.next_char);
	
	write_literals(output, state);
	*buffer_push (*output) = DZIP_CMD_LZ77;
	vluint_write (output, match.distance);
	vluint_write (output, match.length);
	*buffer_push (*output) = match.next_char;

	assert (size == match.length);
	
	for (size_t i = 0; i < size; i++)
	{
	    sliding_window_push (&state->window, shift_input.begin[i]);
	}
    }
    else
    {
	//log_debug("add literal %zu", range_count(state->literals));
	sliding_window_push(&state->window, *shift_input.begin);
	*buffer_push (state->literals) = *shift_input.begin;
	
	size = 1;
    }
    
    /*for (size_t i = 0; i < size; i++)
    {
	sliding_window_push (&state->window, shift_input.begin[i]);
	}*/
    
    state->input_shift += size;

    print_int ("kb/100 to go", range_count(shift_input) / (1024 * 100));
    //print_int ("kb to go", range_count(shift_input) / (1024));
    
    return true;
}
/*
bool dzip_deflate (buffer_unsigned_char * output, dzip_deflate_state * state, const range_const_unsigned_char * input)
{
    assert (state);

    if (input && !range_is_empty (*input))  
    {
	buffer_append(state->input, *input);

	if ((size_t)range_count(state->input) - state->input_shift < state->window.size)
	{
	    return true;
	}
    }
    else
    {
	if (range_count(state->input) - state->input_shift == 0)
	{
	    return false;
	}
    }

    const range_const_unsigned_char shift_input = { .begin = state->input.begin + state->input_shift,
						    .end = state->input.end };

    size_t test_len;

    dzip_match match = {.next_char = *shift_input.begin};
    size_t max_match_length = range_count(shift_input) - 1;
    
    for (size_t i = state->window.size / 2; i < state->window.size && match.length < max_match_length; i--)
    {
	test_len = sliding_window_match_length(&state->window, i, &shift_input);

	if (test_len > match.length)
	{
	    match.length = test_len;
	    match.distance = i;

	    if (match.length > max_match_length)
	    {
		match.length = max_match_length;
	    }
	    
	    match.next_char = shift_input.begin[match.length];
	}

	if (match.length)
	{
	    break;
	}
    }

    print_int ("kb/100 to go", range_count(shift_input) / (1024 * 100));
    
    if (match.length > 30)
    {
    dump_range(&state->input.range_cast.const_cast, 10);
    }

    state->input_shift += match.length + 1;

    if (state->input_shift > 1024)
    {
	buffer_downshift(&state->input, state->input_shift);
	state->input_shift = 0;
    }

    if (match.length > 10)
    {
	log_debug ("(%06zu, %06zu, %03d) %zu", match.distance, match.length, match.next_char, max_match_length);
    }

    sliding_window_push(&state->window, match.next_char);
    
    vluint_write (output, match.distance);
    vluint_write (output, match.length);
    *buffer_push (*output) = match.next_char;

    //printf("size: %zu\n", range_count(*output));
    
    return true;
}
*/

bool dzip_inflate (buffer_unsigned_char * output, dzip_inflate_state * state, const range_const_unsigned_char * input)
{
    return false;
}
