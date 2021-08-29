#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "dzip.h"
#include "../vluint/vluint.h"
#include "internal-shared.h"
#include "../buffer_stream/buffer_stream.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../log/log.h"

#ifndef NDEBUG
#define RECORD_STATS
#endif

typedef uint64_t dzip_match_index;

typedef struct {
    dzip_match_index index;
    dzip_size position;
    dzip_size length;
}
    dzip_match_table_item;

struct dzip_deflate_state {
    dzip_size literal_start;
    bool input_is_finished;
    dzip_size max_chunk_size;

    struct {
	dzip_size window_size;
	dzip_size max_match_length;
	dzip_size skip_count;
	dzip_size match_table_count;
    }
	compression_parameters;

    struct {
	buffer_stream_unsigned_char input;
	dzip_size bytes_read;
	dzip_size bytes_written;
    }
	stream;

#ifdef RECORD_STATS
    struct {
	dzip_size match_count;
	dzip_size match_length;
	dzip_size match_size;
	dzip_size match_offset;
	dzip_size longest_match;
	dzip_size literal_count;
	dzip_size literal_length;
	dzip_size literal_size;
    }
	stats;
#endif
    
    dzip_size recent_new_position[65536];
    
    dzip_size recent_new_start;

    struct range(dzip_match_table_item) match_table;
};

keyargs_define(dzip_deflate_new)
{
    dzip_deflate_state * retval = calloc (1, sizeof(*retval));
    retval->compression_parameters.window_size = args.window_size ? args.window_size : (1e7);
    retval->compression_parameters.max_match_length = args.max_match_length ? args.max_match_length : retval->compression_parameters.window_size;
    retval->compression_parameters.skip_count = args.skip_count;

    retval->compression_parameters.match_table_count = args.match_table_count ? args.match_table_count : 1 + (retval->compression_parameters.window_size) / (sizeof (*retval->match_table.begin));
    retval->compression_parameters.match_table_count = args.match_table_count ? args.match_table_count : retval->compression_parameters.window_size / 10;

    range_calloc (retval->match_table, retval->compression_parameters.match_table_count);

    if (retval->compression_parameters.max_match_length < sizeof (dzip_match_index))
    {
	retval->compression_parameters.max_match_length = sizeof (dzip_match_index);
    }

    retval->max_chunk_size = args.max_chunk_size ? args.max_chunk_size : 1e6;
    
    return retval;
}

static void add_recent_bytes (dzip_deflate_state * state)
{
    range_unsigned_char scan = { .begin = buffer_stream_pointer (state->stream.input, state->recent_new_start),
				 .end = buffer_stream_pointer (state->stream.input, state->stream.bytes_read) };

    scan.end--;
    
    if (!buffer_stream_pointer_check (state->stream.input, scan.end))
    {
	return;
    }

    if (!buffer_stream_pointer_check (state->stream.input, scan.begin))
    {
	assert (false);
	return;
    }

    const unsigned char * i;

    for_range (i, scan)
    {
	state->recent_new_position[*(uint16_t*)i] = buffer_stream_position (state->stream.input, i);
    }

    state->recent_new_start += range_index (i, scan);
}

void dzip_write_header (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    assert (state->stream.bytes_written == 0);

    dzip_size initial_output_size = range_count (*output);

    uint64_t magic = DZIP_MAGIC;
    
    buffer_append_n(*output, &magic, sizeof(magic));

    assert (range_count(*output) - initial_output_size == sizeof(magic));

    buffer_unsigned_char stream_metadata = {0};
    vluint_write(&stream_metadata, state->compression_parameters.window_size);
    vluint_write(output, range_count(stream_metadata));
    buffer_append (*output, stream_metadata);
    free (stream_metadata.begin);

    state->stream.bytes_written += range_count (*output) - initial_output_size;

    //log_debug ("wrote header %.*s", (int)range_count(*output), output->begin);
}

static void write_command (buffer_unsigned_char * output, unsigned char command, vluint_result arg)
{
    assert (command < 4);
    assert (arg > 0);

    unsigned char cmd_byte, arg_byte;
    
    if (arg < 64)
    {
	*buffer_push (*output) = command | (arg << 2);
	
	read_command(&cmd_byte, &arg_byte, output->end[-1]);

	assert (cmd_byte == command);
	assert (arg_byte == arg);
    }
    else
    {
	*buffer_push (*output) = command;
	
	read_command(&cmd_byte, &arg_byte, output->end[-1]);

	assert (cmd_byte == command);
	assert (arg_byte == 0);
	
	vluint_write (output, arg);

	//log_debug ("wrote arg %zu", arg);

	assert (arg > 0);
    }
}

inline static void write_match (buffer_unsigned_char * output, dzip_deflate_state * state, const dzip_match * match)
{
    dzip_size initial_output_size = range_count (*output);
    write_command (output, DZIP_CMD_LZ77, match->length);
    vluint_write (output, match->distance);

    dzip_size added_output_size = range_count (*output) - initial_output_size;

    state->stream.bytes_written += added_output_size;
    state->stream.bytes_read += match->length;
    
#ifdef RECORD_STATS
    state->stats.match_count++;
    state->stats.match_length += match->length;
    state->stats.match_offset += match->distance;
    state->stats.match_size += added_output_size;
	
    if (state->stats.longest_match < match->length)
    {
	state->stats.longest_match = match->length;
    }
#endif
}

static void write_literal (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    range_unsigned_char literal = { .begin = buffer_stream_pointer(state->stream.input, state->literal_start),
				    .end = buffer_stream_pointer(state->stream.input, state->stream.bytes_read) };

    if (range_count (literal) < 1)
    {
	assert (range_count (literal) == 0);
        return;
    }

    if (!buffer_stream_pointer_check (state->stream.input, literal.begin))
    {
	assert (false);
	return;
    }

    if (literal.end > state->stream.input.end)
    {
	assert (false);
	return;
    }

    dzip_size initial_output_size = range_count (*output);

    write_command (output, DZIP_CMD_LITERAL, range_count (literal));
    buffer_append (*output, literal);
	
    state->literal_start = state->stream.bytes_read;

    dzip_size size_difference = range_count(*output) - initial_output_size;

    //log_debug ("wrote literal %zu at offset %zu", range_count (literal), state->stream.bytes_written);

    state->stream.bytes_written += size_difference;

#ifdef RECORD_STATS
    state->stats.literal_length += range_count (literal);
    state->stats.literal_size += size_difference;
    state->stats.literal_count++;
#endif
}

static void setup_match (dzip_match * match, const range_const_unsigned_char * scan_region, dzip_size distance, unsigned char skip)
{
    match->distance = distance;
    
    const unsigned char * i = scan_region->begin + skip;

    while (i < scan_region->end)
    {
	if (*i != *(i - distance))
	{
	    break;
	}
	i++;
    }
    
    /*for_range (i, *scan_region)
      {
      if (*i != *(i - distance))
      {
      break;
      }
      }*/

    match->length = range_index (i, *scan_region);
}

inline static dzip_size match_offset_max (dzip_deflate_state * state)
{
    const unsigned char * start = buffer_stream_pointer(state->stream.input, state->stream.bytes_read);
    const unsigned char * window_min = start - state->compression_parameters.window_size;

    if (!buffer_stream_pointer_check (state->stream.input, start))
    {
	assert (start > state->stream.input.begin);
	assert (false);
	return 0;
    }
    
    if (window_min > state->stream.input.begin)
    {
	return state->compression_parameters.window_size;
    }
    else
    {
	return start - state->stream.input.begin;
    }
}

inline static void match_offset_scan_region (range_const_unsigned_char * scan_region, dzip_deflate_state * state)
{
    scan_region->begin = buffer_stream_pointer(state->stream.input, state->stream.bytes_read);
    scan_region->end = scan_region->begin + state->compression_parameters.max_match_length;

    if (scan_region->end > state->stream.input.end - 1)
    {
	scan_region->end = state->stream.input.end - 1;
    }

    assert (scan_region->begin >= state->stream.input.begin);
    assert (scan_region->end < state->stream.input.end);
}

static void find_match (dzip_match * match, dzip_deflate_state * state)
{
    dzip_size max_ofs = match_offset_max (state);

    if (max_ofs == 0)
    {
	goto no_match;
    }

    range_const_unsigned_char scan_region = {0};

    match_offset_scan_region (&scan_region, state);

    if (state->input_is_finished)
    {
	assert (state->compression_parameters.max_match_length >= sizeof(dzip_match_index));
	if ((size_t) range_count (scan_region) < state->compression_parameters.max_match_length)
	{
	    goto no_match;
	}
    }
    else
    {
	if ((size_t) range_count (scan_region) < sizeof(dzip_match_index))
	{
	    goto no_match;
	}
    }
    
    add_recent_bytes (state);
    
    uint16_t byte_index = *(uint16_t*)scan_region.begin;

    dzip_size recent_new_position = state->recent_new_position[byte_index];

    if (!recent_new_position)
    {
	goto no_match;
    }
    
    dzip_size min_ofs_new = state->stream.bytes_read - recent_new_position;

    if (min_ofs_new > max_ofs)
    {
	goto no_match;
    }

    //assert (*(dzip_match_index*) (scan_region.begin - min_ofs_new) == byte_index);

    dzip_match match_new;
    setup_match(&match_new, &scan_region, min_ofs_new, 0);

    dzip_match_index match_index = *(dzip_match_index*)scan_region.begin;
    dzip_match_table_item * item = state->match_table.begin + match_index % range_count (state->match_table);

    if (item->index != match_index || item->length < match_new.length)
    {
	goto use_new;
    }

    dzip_size min_ofs_table = state->stream.bytes_read - item->position;

    if (min_ofs_table > max_ofs)
    {
	goto use_new;
    }

    dzip_match match_table;
    assert (*(dzip_match_index*)(scan_region.begin - min_ofs_table) == match_index);
    setup_match (&match_table, &scan_region, min_ofs_table, sizeof(dzip_match_index));

    if (match_table.length > match_new.length)
    {
	goto use_table;
    }
    else
    {
	goto use_new;
    }
    
    assert (false);

use_new:
    //log_debug ("new match: %zu %zu", match_new.length, match_new.distance);
    *match = match_new;
    
    if (match->length >= sizeof(dzip_match_index))
    {
	item->index = match_index;
	item->length = match->length;
	item->position = state->stream.bytes_read - match->distance;
    }
    
    assert (match->distance <= state->compression_parameters.window_size);
    assert (match->length <= state->compression_parameters.max_match_length);

    return;

use_table:
    *match = match_table;

    return;
    
no_match:
    *match = (dzip_match) {};
    return;
}

bool dzip_deflate_update (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    if (state->stream.bytes_written == 0)
    {
	dzip_write_header(output, state);
	//return true;
    }

    dzip_match match = {0};

    const unsigned char * new_input;
	
    while (true)
    {
	new_input = buffer_stream_pointer (state->stream.input, state->stream.bytes_read);
	
	if (new_input == state->stream.input.end)
	{
	    if (state->input_is_finished)
	    {
		write_literal(output, state);
	    }
	    
	    break;
	}

	if (dzip_deflate_is_full (output, state))
	{
	    break;
	}

	//if (false)
	find_match (&match, state);
	
	if (4 < match.length && match.distance > match.length)
	{
	    write_literal (output, state);
	    write_match (output, state, &match);
	    state->literal_start = state->stream.bytes_read;
	}
	else
	{
	    state->stream.bytes_read++;
	}
    }

    assert (new_input >= state->stream.input.begin);
    /*size_t past_size = new_input - state->stream.input.begin;
	
    if (past_size > 2 * state->compression_parameters.window_size && past_size > (size_t) range_count(state->stream.input) / 2)
    {
	write_literal(output, state);
	add_recent_bytes(state);

	assert (new_input - past_size >= state->stream.input.begin);

	size_t prune_size = past_size - state->compression_parameters.window_size;
	buffer_stream_shift(state->stream.input, prune_size);
	}*/

    return false;
}
    
buffer_unsigned_char * dzip_deflate_input_buffer (dzip_deflate_state * state)
{
    return &state->stream.input.buffer_cast;
}
    
void dzip_deflate_eof (dzip_deflate_state * state)
{
    state->input_is_finished = true;
}

void dzip_deflate_free (dzip_deflate_state * state)
{
    free (state->stream.input.begin);
    free (state);
}

bool dzip_deflate_is_full (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    return (size_t) range_count (*output) > state->max_chunk_size;
}

bool dzip_deflate_is_done (dzip_deflate_state * state)
{
    return (state->input_is_finished) && (state->stream.input.end == buffer_stream_pointer (state->stream.input, state->stream.bytes_read));
}

void dzip_deflate_print_stats (dzip_deflate_state * state)
{
    log_stderr ("Output/input ratio total: %lf", (double)state->stream.bytes_written /(double)state->stream.bytes_read);

#ifdef RECORD_STATS
    log_stderr ("Output/input ratio literal: %lf", (double)state->stats.literal_size / (double)state->stats.literal_length);
    log_stderr ("Output/input ratio match: %lf", (double)state->stats.match_size / (double)state->stats.match_length);

    log_stderr ("");
    
    log_stderr ("Average literal size: %lf", (double)state->stats.literal_size / (double)state->stats.literal_count);
    log_stderr ("Average match size: %lf", (double)state->stats.match_size / (double)state->stats.match_count);

    log_stderr ("");

    log_stderr ("Average literal length: %lf", (double)state->stats.literal_length / (double)state->stats.literal_count);
    log_stderr ("Average match length: %lf", (double)state->stats.match_length / (double)state->stats.match_count);

    log_stderr ("");
    
    log_stderr ("Literal weight in output: %lf", (double)state->stats.literal_size / (double)state->stream.bytes_written);
    log_stderr ("Literal weight in input: %lf", (double)state->stats.literal_length / (double)state->stream.bytes_read);
    log_stderr ("Match weight in output: %lf", (double)state->stats.match_size / (double)state->stream.bytes_written);
    log_stderr ("Match weight in input: %lf", (double)state->stats.match_length / (double)state->stream.bytes_read);
    
    log_stderr ("");

    log_stderr ("Average match offset: %lf", (double)state->stats.match_offset / (double)state->stats.match_count);
    log_stderr ("Average match offset as a percent of the window: %lf", ((double)state->stats.match_offset / (double)state->stats.match_count) / (double)state->compression_parameters.window_size);
    log_stderr ("Longest match: %zu", state->stats.longest_match);
    log_stderr ("Match / Literal size ratio: %lf", (double)state->stats.match_size / (double)state->stats.literal_size);
#endif
    
    log_stderr ("Max match length: %zu", state->compression_parameters.max_match_length);
    log_stderr ("Skip rate: %zu", state->compression_parameters.skip_count);

    log_debug ("State size: %lf mb", (double) sizeof(dzip_deflate_state) / (double) 1e6);
    log_debug ("Window size: %lf mb", (double) state->compression_parameters.window_size / (double) 1e6);
    log_debug ("Match table size: %lf mb", (double) (sizeof(*state->match_table.begin) * range_count(state->match_table)) / (double) 1e6);
}
