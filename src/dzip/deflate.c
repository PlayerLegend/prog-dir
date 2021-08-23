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

struct dzip_deflate_state {
        
    //dzip_size input_shift_total;
    dzip_size literal_start;
    //buffer_unsigned_char input;
    bool input_is_finished;

    struct {
	dzip_size window_size;
	dzip_size max_match_length;
	dzip_size skip_count;
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
	dzip_size repeat_length;
	dzip_size repeat_count;
	dzip_size repeat_size;
    }
	stats;
#endif
    
    dzip_size recent_new_position[65536];
    dzip_size recent_old_position[65536];
    
    dzip_size recent_new_start;
};

keyargs_define(dzip_deflate_new)
{
    dzip_deflate_state * retval = calloc (1, sizeof(*retval));
    retval->compression_parameters.window_size = args.window_size ? args.window_size : (1.2e6);
    retval->compression_parameters.max_match_length = args.max_match_length ? args.max_match_length : retval->compression_parameters.window_size;
    retval->compression_parameters.skip_count = args.skip_count;
    
    //log_debug ("window size: %zu", retval->window_size);
    
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
/*
static dzip_size count_repeat (const range_const_unsigned_char * shift_input)
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
*/

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
/*
inline static dzip_size write_repeat (buffer_unsigned_char * output, size_t size, unsigned char c)
{
    dzip_size initial_output_size = range_count (*output);
    write_command (output, DZIP_CMD_REPEAT, size);
    *buffer_push (*output) = c;
    return range_count (*output) - initial_output_size;
}
*/

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

inline static dzip_size count_dead_bytes (dzip_deflate_state * state)
{
    const unsigned char * window_min = buffer_stream_pointer (state->stream.input, state->stream.bytes_read - state->compression_parameters.window_size);
    const unsigned char * literal_min = buffer_stream_pointer (state->stream.input, state->literal_start);
    const unsigned char * recent_min = buffer_stream_pointer (state->stream.input, state->recent_new_start);

    const unsigned char * overall_min = window_min;

    if (overall_min > literal_min)
    {
	overall_min = literal_min;
    }

    if (overall_min > recent_min)
    {
	overall_min = recent_min;
    }

    if (overall_min > state->stream.input.end)
    {
	assert (false);
	return 0;
    }

    if (overall_min > state->stream.input.begin)
    {
	return range_index(overall_min, state->stream.input);
    }
    else
    {
	return 0;
    }
}

inline static void prune_dead_bytes (dzip_deflate_state * state)
{
    dzip_size dead_bytes = count_dead_bytes (state);

    if (dead_bytes > (dzip_size)range_count (state->stream.input) / 2 && dead_bytes > 1e6)
    {
	buffer_stream_shift (state->stream.input, dead_bytes);
    }
}

static void setup_match (dzip_match * match, const range_const_unsigned_char * scan_region, dzip_size distance)
{
    match->distance = distance;
    
    const unsigned char * i;
    
    for_range (i, *scan_region)
    {
	if (*i != *(i - distance))
	{
	    break;
	}
    }

    match->length = range_index (i, *scan_region);
}

static dzip_size match_offset_max (dzip_deflate_state * state)
{
    const unsigned char * start = buffer_stream_pointer(state->stream.input, state->stream.bytes_read);
    const unsigned char * window_min = start - state->compression_parameters.window_size;

    if (!buffer_stream_pointer_check (state->stream.input, start))
    {
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

static void match_offset_scan_region (range_const_unsigned_char * scan_region, dzip_deflate_state * state)
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
	assert (state->compression_parameters.max_match_length >= 2);
	if ((size_t) range_count (scan_region) < state->compression_parameters.max_match_length)
	{
	    goto no_match;
	}
    }
    else
    {
	if (range_count (scan_region) < 2)
	{
	    goto no_match;
	}
    }
    
    add_recent_bytes (state);
    
    uint16_t byte_index = *(uint16_t*)scan_region.begin;

    scan_region.begin += 2; // WATCH OUT
    
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

    dzip_match match_new;
    setup_match(&match_new, &scan_region, min_ofs_new);

    dzip_size min_ofs_old = state->stream.bytes_read - state->recent_old_position[byte_index];

    if (min_ofs_old == min_ofs_new)
    {
	goto use_new;
    }

    if (min_ofs_old > max_ofs)
    {
	goto use_new;
    }

    dzip_match match_old;

    setup_match (&match_old, &scan_region, min_ofs_old);

    if (match_old.length > match_new.length)
    {
	goto use_old;
    }
    else
    {
	goto use_new;
    }
    
    assert (false);

use_new:
    //log_debug ("new match: %zu %zu", match_new.length, match_new.distance);
    *match = match_new;
    state->recent_old_position[byte_index] = state->stream.bytes_read - match->distance;
    
    assert (match->distance <= state->compression_parameters.window_size);
    assert (match->length <= state->compression_parameters.max_match_length);

    return;

use_old:
    //log_debug ("old match: %zu %zu", match_new.length, match_new.distance);
    *match = match_old;
    return;
    
no_match:
    *match = (dzip_match) {};
    return;
}

bool dzip_deflate_update (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    if (state->stream.bytes_written == 0)
    {
	//log_debug ("write header");
	dzip_write_header(output, state);
	return true;
    }
    
    prune_dead_bytes (state);

    if (state->stream.input.end == buffer_stream_pointer (state->stream.input, state->stream.bytes_read))
    {
	if (state->input_is_finished)
	{
	    write_literal(output, state);
	}

	return false;
    }

    dzip_match match = {0};

    //if (false)
	find_match (&match, state);

    if (4 < match.length)
    {
	write_literal (output, state);
	write_match (output, state, &match);
	//log_debug ("here %zu %zu", state->stream.bytes_written, state->stream.bytes_read);
	state->literal_start = state->stream.bytes_read;
    }
    else
    {
	state->stream.bytes_read++;
    }

    return true;
}

void dzip_deflate_print_stats (dzip_deflate_state * state)
{
    log_stderr ("Output/input ratio total: %lf", (double)state->stream.bytes_written /(double)state->stream.bytes_read);

#ifdef RECORD_STATS
    log_stderr ("Output/input ratio literal: %lf", (double)state->stats.literal_size / (double)state->stats.literal_length);
    log_stderr ("Output/input ratio repeat: %lf", (double)state->stats.repeat_size / (double)state->stats.repeat_length);
    log_stderr ("Output/input ratio match: %lf", (double)state->stats.match_size / (double)state->stats.match_length);

    log_stderr ("");
    
    log_stderr ("Average literal size: %lf", (double)state->stats.literal_size / (double)state->stats.literal_count);
    log_stderr ("Average repeat size: %lf", (double)state->stats.repeat_size / (double)state->stats.repeat_count);
    log_stderr ("Average match size: %lf", (double)state->stats.match_size / (double)state->stats.match_count);

    log_stderr ("");

    log_stderr ("Average literal length: %lf", (double)state->stats.literal_length / (double)state->stats.literal_count);
    log_stderr ("Average repeat length: %lf", (double)state->stats.repeat_length / (double)state->stats.repeat_count);
    log_stderr ("Average match length: %lf", (double)state->stats.match_length / (double)state->stats.match_count);

    log_stderr ("");
    
    log_stderr ("Literal weight in output: %lf", (double)state->stats.literal_size / (double)state->stream.bytes_written);
    log_stderr ("Literal weight in input: %lf", (double)state->stats.literal_length / (double)state->stream.bytes_read);
    log_stderr ("Repeat weight in output: %lf", (double)state->stats.repeat_size / (double)state->stream.bytes_written);
    log_stderr ("Repeat weight in input: %lf", (double)state->stats.repeat_length / (double)state->stream.bytes_read);
    log_stderr ("Match weight in output: %lf", (double)state->stats.match_size / (double)state->stream.bytes_written);
    log_stderr ("Match weight in input: %lf", (double)state->stats.match_length / (double)state->stream.bytes_read);
    
    log_stderr ("");

    log_stderr ("Average match offset: %lf", (double)state->stats.match_offset / (double)state->stats.match_count);
    log_stderr ("Average match offset as a percent of the window: %lf", ((double)state->stats.match_offset / (double)state->stats.match_count) / (double)state->compression_parameters.window_size);
    log_stderr ("Longest match: %zu", state->stats.longest_match);
    log_stderr ("Match / Literal size ratio: %lf", (double)state->stats.match_size / (double)state->stats.literal_size);
#endif
    
    log_stderr ("Window size: %zu", state->compression_parameters.window_size);
    log_stderr ("Max match length: %zu", state->compression_parameters.max_match_length);
    log_stderr ("Skip rate: %zu", state->compression_parameters.skip_count);
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
