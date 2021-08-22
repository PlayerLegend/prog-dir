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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../log/log.h"

struct dzip_deflate_state {
    dzip_size window_size;
    dzip_size max_match_length;
    //dzip_size input_index;
    dzip_size input_shift_total;
    dzip_size literal_start;
    buffer_unsigned_char input;
    bool input_is_finished;
    dzip_size skip_count;
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
	dzip_size bytes_read;
	dzip_size bytes_written;
    }
	stats;

    dzip_size recent_new_position[65536];
    dzip_size recent_old_position[65536];
    
    dzip_size recent_new_start;
};

keyargs_define(dzip_deflate_new)
{
    dzip_deflate_state * retval = calloc (1, sizeof(*retval));
    retval->window_size = args.window_size ? args.window_size : (sizeof(dzip_deflate_state));
    retval->max_match_length = args.max_match_length ? args.max_match_length : retval->window_size;
    retval->skip_count = args.skip_count;
    
    //log_debug ("window size: %zu", retval->window_size);
    
    return retval;
}

static void add_recent_bytes (dzip_deflate_state * state)
{
    dzip_size start = state->recent_new_start - state->input_shift_total;

    assert (state->recent_new_start >= state->input_shift_total);
    
    /*
    dzip_size end = state->stats.bytes_read - state->input_shift_total;
    
    assert (state->stats.bytes_read >= state->input_shift_total);*/

    //dzip_size count = state->stats.bytes_read - 1 - state->recent_bytes_start;
    dzip_size count = state->stats.bytes_read - state->input_shift_total - start;

    assert (count < (size_t)range_count (state->input));

    if (!count)
    {
	return;
    }

    count--;
    
    /*if (count > state->stats.bytes_read)
    {
	return;
	}*/
    
    //assert (state->stats.bytes_read >= state->recent_bytes_start);

    for (dzip_size i = 0; i < count; i++)
    {
	//state->recent_bytes[state->input.begin[start + i]] = state->recent_bytes_start + i;
	state->recent_new_position[*(uint16_t*)(state->input.begin + start + i)] = state->recent_new_start + i;
	//log_debug ("set %zu", *(uint16_t*)(state->input.begin + start + i));
    }

    //log_debug ("added %zu", count);

    state->recent_new_start += count;
}

void dzip_write_header (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    assert (state->stats.bytes_written == 0);

    dzip_size initial_output_size = range_count (*output);

    uint64_t magic = DZIP_MAGIC;
    
    buffer_append_n(*output, &magic, sizeof(magic));

    assert (range_count(*output) - initial_output_size == sizeof(magic));

    buffer_unsigned_char stream_metadata = {0};
    vluint_write(&stream_metadata, state->window_size);
    vluint_write(output, range_count(stream_metadata));
    buffer_append (*output, stream_metadata);
    free (stream_metadata.begin);

    state->stats.bytes_written += range_count (*output) - initial_output_size;

    //log_debug ("wrote header %.*s", (int)range_count(*output), output->begin);
}

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

	assert (arg > 0);
    }
}

static bool write_literals (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    range_unsigned_char literal = { .begin = state->input.begin + state->literal_start - state->input_shift_total,
				    .end = state->input.begin + state->stats.bytes_read - state->input_shift_total };

    if (!range_is_empty (literal))
    {
	assert (range_count (literal) <= range_count(state->input));

	state->stats.literal_count++;
	state->stats.literal_length += range_count (literal);

	dzip_size initial_output_size = range_count (*output);

	write_command (output, DZIP_CMD_LITERAL, range_count (literal));
	buffer_append (*output, literal);
	
	state->literal_start = state->stats.bytes_read;
	
	dzip_size size_difference = range_count(*output) - initial_output_size;

	state->stats.bytes_written += size_difference;
	state->stats.literal_size += size_difference;

	//log_debug ("wrote literal %zu", range_count(literal));

	return true;
    }
    else
    {
	return false;
    }
}

inline static dzip_size count_dead_bytes (dzip_deflate_state * state)
{
    dzip_size bytes_count = state->stats.bytes_read - state->input_shift_total;

    assert (0 < range_count (state->input));
    assert (bytes_count <= (dzip_size)range_count(state->input));

    dzip_size window_reserve = state->window_size;
    dzip_size literal_reserve = state->stats.bytes_read - state->literal_start;

    assert (state->stats.bytes_read >= state->literal_start);
    assert (literal_reserve <= bytes_count);

    dzip_size reserve = window_reserve > literal_reserve ? window_reserve : literal_reserve;

    if (reserve > bytes_count)
    {
	reserve = bytes_count;
    }

    return bytes_count - reserve;
}

inline static void prune_dead_bytes (dzip_deflate_state * state)
{
    dzip_size dead_bytes = count_dead_bytes (state);

    assert (dead_bytes <= state->stats.bytes_read - state->input_shift_total);

    assert (state->stats.bytes_read >= state->input_shift_total);
    assert (state->input.begin + state->stats.bytes_read - state->input_shift_total <= state->input.end);
	
    //add_recent_bytes (state);
    
    if (dead_bytes > (dzip_size)range_count (state->input) / 2 && dead_bytes > 1e6)
    {
	if (state->input_shift_total + dead_bytes > state->recent_new_start)
	{
	    add_recent_bytes (state);
	    return;
	}
	
	//log_debug ("prune");
	buffer_downshift(state->input, dead_bytes);
	state->input_shift_total += dead_bytes;
    }

    /*if (state->stats.bytes_read < state->input_shift_total)
      {
      log_debug ("Bad: %zu < %zu", state->stats.bytes_read, state->input_shift_total);
      }*/
    
    assert (state->stats.bytes_read >= state->input_shift_total);	
    assert ((long long int)(state->stats.bytes_read - state->input_shift_total) <= range_count (state->input));
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

    match->next_char = *i;

    match->length = range_index (i, *scan_region);
}

/*static void setup_match (dzip_match * match, const range_const_unsigned_char * scan_region, dzip_size distance)
{
    match->distance = distance;
    
    const unsigned char * i = scan_region->begin + 2;

    while (i < scan_region->end)
    {
	if (*i != *(i - distance))
	{
	    break;
	}
	
	i++;
    }
    
    match->next_char = *i;

    match->length = range_index (i, *scan_region);
    }*/

/*static void setup_match (dzip_match * match, const range_const_unsigned_char * scan_region, dzip_size distance)
{
    match->distance = distance;
    
    const unsigned char * i = scan_region->begin;
    
    dzip_size length_2 = range_count (*scan_region) / 2;

    const unsigned char * max2 = i + 2 * length_2;

    while (i < max2)
    {
	if ( *(uint16_t*)i != *(uint16_t*)(i - distance) )
	{
	    break;
	}
	
	i += 2;
    }

    log_debug ("i8: %zu", i - scan_region->begin);
    
    while (i < scan_region->end)
    {
	if (*i != *(i - distance))
	{
	    break;
	}

	i++;
    }
    
    match->next_char = *i;

    match->length = range_index (i, *scan_region);
}*/

static dzip_size match_offset_max (dzip_deflate_state * state)
{
    assert (state->stats.bytes_read >= state->input_shift_total);
    dzip_size input_start = state->stats.bytes_read - state->input_shift_total;
    assert (range_count (state->input) > 0);
    assert (input_start < (dzip_size)range_count (state->input));
    
    return input_start > state->window_size ? state->window_size : input_start;
}

static void match_offset_scan_region (range_const_unsigned_char * scan_region, dzip_deflate_state * state)
{
    scan_region->begin = state->input.begin - state->input_shift_total + state->stats.bytes_read;
    scan_region->end = scan_region->begin + state->max_match_length;

    if (scan_region->end > state->input.end - 1)
    {
	scan_region->end = state->input.end - 1;
    }

    assert (scan_region->begin >= state->input.begin);
    assert (scan_region->end < state->input.end);
}

static dzip_size match_offset (dzip_match * match, dzip_deflate_state * state)
{
    dzip_size max_ofs = match_offset_max (state);

    if (max_ofs == 0)
    {
	goto no_match;
    }

    range_const_unsigned_char scan_region = {0};

    match_offset_scan_region (&scan_region, state);

    if (range_count (scan_region) < 2)
    {
	goto no_match;
    }
    
    add_recent_bytes (state);
    
    uint16_t byte_index = *(uint16_t*)scan_region.begin;

    scan_region.begin += 2; // WATCH OUT
    
    dzip_size recent_new_position = state->recent_new_position[byte_index];

    if (!recent_new_position)
    {
	goto no_match;
    }
    
    dzip_size min_ofs_new = state->stats.bytes_read - recent_new_position;

    if (min_ofs_new > max_ofs)
    {
	goto no_match;
    }

    dzip_match match_new;
    setup_match(&match_new, &scan_region, min_ofs_new);

    dzip_size min_ofs_old = state->stats.bytes_read - state->recent_old_position[byte_index];

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

    //goto use_new;
    
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
    state->recent_old_position[byte_index] = state->stats.bytes_read - match->distance;
    
    assert (match->distance <= state->window_size);
    assert (match->length <= state->max_match_length);

    return match->length;

use_old:
    //log_debug ("old match: %zu %zu", match_new.length, match_new.distance);
    *match = match_old;
    return match->length;
    
no_match:
    *match = (dzip_match) { .next_char = state->input.begin[state->stats.bytes_read - state->input_shift_total] };
    return 0;
}

inline static dzip_size write_repeat (buffer_unsigned_char * output, size_t size, unsigned char c)
{
    dzip_size initial_output_size = range_count (*output);
    write_command (output, DZIP_CMD_REPEAT, size);
    *buffer_push (*output) = c;
    return range_count (*output) - initial_output_size;
}

inline static dzip_size write_match (buffer_unsigned_char * output, const dzip_match * match)
{
    dzip_size initial_output_size = range_count (*output);
    write_command (output, DZIP_CMD_LZ77, match->length);
    vluint_write (output, match->distance);
    *buffer_push (*output) = match->next_char;
    return range_count (*output) - initial_output_size;
}

bool dzip_deflate_update (buffer_unsigned_char * output, dzip_deflate_state * state)
{
    dzip_size added_output_size = 0;

    //log_debug ("bytes written: %zu", state->stats.bytes_written);
    //exit (1);
    
    if (state->stats.bytes_written == 0)
    {
	dzip_write_header(output, state);
	return true;
    }
    
    prune_dead_bytes (state);

    const range_const_unsigned_char input = { .begin = state->input.begin + state->stats.bytes_read - state->input_shift_total,
					      .end = state->input.end };

    if (state->input_is_finished)
    {
	if (range_is_empty (input))
	{
	    if (write_literals(output, state))
	    {
		return true;
	    }
	    else
	    {
		return false;
	    }
	}
    }
    else
    {
	if ((dzip_size)range_count (input) < state->max_match_length)
	{
	    return false;
	}
    }
    
    assert (range_count (input) > 0);
    
    dzip_match match;
    dzip_size size;
    //dzip_size metadata_threshold_size = 4; //5;

    //dzip_size literal_length = state->stats.bytes_read - state->literal_start;

    //bool preserve_literal = literal_length > 0 && literal_length < 5;
    
    assert (input.begin < state->input.end);

    if (2 < (size = count_repeat (&input)))
    {
	write_literals(output, state);
	added_output_size = write_repeat(output, size, *input.begin);

	state->stats.bytes_read += size;
	state->stats.bytes_written += added_output_size;
	
	state->stats.repeat_count++;
	state->stats.repeat_length += size;
	state->stats.repeat_size += added_output_size;
	
	state->literal_start = state->stats.bytes_read;
	
	//log_debug ("wrote repeat %d[%zu]", *input.begin, size);
    }
    else if (4 < (size = match_offset (&match, state)))
    {
	assert (size == match.length);
	
	write_literals(output, state);

	added_output_size = write_match (output, &match);
        
	state->stats.bytes_read += size;
	state->stats.bytes_written += added_output_size;
	
	state->stats.match_count++;
	state->stats.match_length += match.length;
	state->stats.match_offset += match.distance;
	state->stats.match_size += added_output_size;
	
	if (state->stats.longest_match < match.length)
	{
	    state->stats.longest_match = match.length;
	}
	
	state->literal_start = state->stats.bytes_read;
	
	//log_debug ("added match (%06zu, %06zu, %03d)", match.distance, match.length, match.next_char);
	//log_debug ("match: %.*s", match.length, input.begin - match.distance);
    }
    else
    {
	dzip_size skip_size = 1 + state->skip_count;

	size = range_count (input);
	
	if (size > skip_size)
	{
	    size = skip_size;
	}
	
	state->stats.bytes_read += size;
    }

    //print_int ("mb to go", range_count(input) / (1024 * 1024));

    assert (state->input.begin + state->stats.bytes_read - state->input_shift_total <= state->input.end);
    assert ((long long int)(state->stats.bytes_read - state->input_shift_total) <= range_count(state->input));
    
    return true; //state->stats.bytes_read - state->input_shift_total < (dzip_size)range_count(state->input);
}

void dzip_deflate_print_stats (dzip_deflate_state * state)
{
    log_stderr ("Output/input ratio total: %lf", (double)state->stats.bytes_written /(double)state->stats.bytes_read);
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
    
    log_stderr ("Literal weight in output: %lf", (double)state->stats.literal_size / (double)state->stats.bytes_written);
    log_stderr ("Literal weight in input: %lf", (double)state->stats.literal_length / (double)state->stats.bytes_read);
    log_stderr ("Repeat weight in output: %lf", (double)state->stats.repeat_size / (double)state->stats.bytes_written);
    log_stderr ("Repeat weight in input: %lf", (double)state->stats.repeat_length / (double)state->stats.bytes_read);
    log_stderr ("Match weight in output: %lf", (double)state->stats.match_size / (double)state->stats.bytes_written);
    log_stderr ("Match weight in input: %lf", (double)state->stats.match_length / (double)state->stats.bytes_read);
    
    log_stderr ("");

    log_stderr ("Average match offset: %lf", (double)state->stats.match_offset / (double)state->stats.match_count);
    log_stderr ("Average match offset as a percent of the window: %lf", ((double)state->stats.match_offset / (double)state->stats.match_count) / (double)state->window_size);
    log_stderr ("Longest match: %zu", state->stats.longest_match);
    log_stderr ("Match / Literal size ratio: %lf", (double)state->stats.match_size / (double)state->stats.literal_size);
    log_stderr ("Window size: %zu", state->window_size);
    log_stderr ("Max match length: %zu", state->max_match_length);
    log_stderr ("Skip rate: %zu", state->skip_count);
}

buffer_unsigned_char * dzip_deflate_input_buffer (dzip_deflate_state * state)
{
    return &state->input;
}

void dzip_deflate_eof (dzip_deflate_state * state)
{
    state->input_is_finished = true;
}

void dzip_deflate_free (dzip_deflate_state * state)
{
    free (state->input.begin);
    free (state);
}
