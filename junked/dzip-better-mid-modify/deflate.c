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

typedef uint64_t dzip_best_index;

#define count_array(array) (sizeof(array) / sizeof((array)[0]))

typedef union {
    char bytes[8];
    
    struct {
	char _reserve_byte[7];
	char byte;
    };
	
    struct {
	uint16_t _reserve_i16[3];
	uint16_t i16;
    };
	
    uint64_t i64;
}
    dzip_scrolling_index;

struct dzip_deflate_state {
    uint64_t window[DZIP_ARG1_MAX];
    buffer_char literal;
    dzip_size bytes_read;
    dzip_size bytes_rest;
    bool header_is_written;
    dzip_scrolling_index index;
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
    if (!match->length)
    {
	return;
    }
    
#ifdef DZIP_RECORD_STATS
    stat_match_count++;
    stat_match_length += match->length;
#endif
    
    log_debug ("writing match %zu %zu", match->length, match->point);
    write_command (output, DZIP_CMD_MATCH, match->length);
    write_point(output, match->point);
}

static void write_literal (buffer_char * restrict output, dzip_size length, char * restrict begin)
{
    if (!length)
    {
	return;
    }
    
#ifdef DZIP_RECORD_STATS
    stat_literal_count++;
    stat_literal_length += length;
#endif

    log_debug ("writing literal %zu", length);
    write_command (output, DZIP_CMD_LITERAL, length);
    buffer_append_n (*output, begin, length);
}

inline static void add_index_byte (dzip_scrolling_index * restrict index, char byte)
{
    index->i64 >>= sizeof(index->byte);
    index->byte = byte;
}

keyargs_define(dzip_deflate)
{
    //range_const_char input = *args.input;
    const char * restrict begin = args.input->begin;
    const char * restrict end = args.input->end;

    if (!args.state->header_is_written)
    {
	buffer_append_n (*args.output, "deezwhat", 8);
	args.state->header_is_written = true;
    }

    if (false)
	add_byte (args.output, args.state, *begin);

    while (begin < end)
    {
	//log_debug ("to-go: %zu", range_count(input));
	//add_byte (args.output, args.state, *input.begin);
	add_index_byte (&args.state->index, *begin);
	begin++;
    }

    //write_literal (args.output, &args.state->cache);
    
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
