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
    stat_match4_count,
    stat_match8_count,
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

    struct {
	uint32_t _reserve_i32;
	uint32_t i32;
    };
    
    uint64_t i64;
}
    dzip_scrolling_index;

struct dzip_deflate_state {
    uint64_t window[DZIP_ARG1_MAX - 1];
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

static void write_match4 (buffer_char * restrict output, dzip_deflate_state * state, uint32_t * point_p)
{
#ifdef DZIP_RECORD_STATS
    stat_match4_count++;
#endif
    
    //log_debug ("writing match4 %zu = %zu", (uint64_t*) point_p - state->window, *point_p);
    write_command (output, DZIP_CMD_MATCH4, (uint64_t*) point_p - state->window);
    state->bytes_rest = state->bytes_read + 4;
}

static void write_match8 (buffer_char * restrict output, dzip_deflate_state * state, uint64_t * point_p)
{
#ifdef DZIP_RECORD_STATS
    stat_match8_count++;
#endif
    
    //log_debug ("writing match8 %zu = %zu", point_p - state->window, *point_p);
    write_command (output, DZIP_CMD_MATCH8, point_p - state->window);
    state->bytes_rest = state->bytes_read + 8;
}

static void write_literal (buffer_char * restrict output, dzip_deflate_state * state)
{
    dzip_size length = range_count(state->literal);
    
    if (!length)
    {
	return;
    }
    
#ifdef DZIP_RECORD_STATS
    stat_literal_count++;
    stat_literal_length += length;
#endif

    //log_debug ("writing literal %zu", length);
    write_command (output, DZIP_CMD_LITERAL, length);
    buffer_append_n (*output, state->literal.begin, length);
    //state->bytes_rest = state->bytes_read + length;
    buffer_rewrite (state->literal);
}

inline static void add_index_byte (dzip_scrolling_index * restrict index, char byte)
{
    index->i64 >>= sizeof(index->byte);
    index->byte = byte;
}

inline static uint64_t * try_match8 (dzip_deflate_state * state)
{
    //log_debug ("match8(%zu): %zu", state->index.i64, state->index.i64 % count_array(state->window));
    return state->window + state->index.i64 % count_array(state->window);
}

inline static uint32_t * try_match4 (dzip_deflate_state * state)
{
    //log_debug ("match4(%zu): %zu", state->index.i32, state->index.i32 % count_array(state->window));
    return (uint32_t*)(state->window + state->index.i32 % count_array(state->window));
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
	assert (args.state->bytes_rest == 0);
	assert (args.state->bytes_read == 0);
	args.state->bytes_rest = 4;
	log_debug ("write header");
    }

    uint64_t * match8;
    uint32_t * match4;

    for (begin = args.input->begin, end = args.input->end; begin < end; begin++)
    {
	//log_debug ("tick");
	
	add_index_byte (&args.state->index, *begin);
	args.state->bytes_read++;

	if (args.state->bytes_rest > args.state->bytes_read)
	{
	    //log_debug ("rest %zu", args.state->bytes_rest - args.state->bytes_read);
	    continue;
	}
	else if (*(match8 = try_match8 (args.state)) == args.state->index.i64)
	{
	    write_literal(args.output, args.state);
	    assert (*match8 == args.state->index.i64);
	    write_match8(args.output, args.state, match8);
	}
	else if (*(match4 = try_match4 (args.state)) == args.state->index.i32)
	{
	    write_literal(args.output, args.state);
	    assert (*match4 == args.state->index.i32);
	    write_match4(args.output, args.state, match4);
	}
	else
	{
	    if (range_count(args.state->literal) == DZIP_ARG1_MAX - 1)
	    {
		write_literal(args.output, args.state);
	    }
		
	    *buffer_push (args.state->literal) = args.state->index.bytes[0];
	    //log_debug ("add literal byte %zu", range_count(args.state->literal));
	    //log_debug ("literal byte: %u", args.state->literal.end[-1]);
	    *match4 = args.state->index.i32;
	    //*match8 = args.state->index.i64;
	}
	
    }
    
    write_literal(args.output, args.state);
    
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
    log_stderr ("Match4 weight in input = %zu / %zu = %lf", stat_match4_count * 4, stat_bytes_in, (double) (stat_match4_count * 4) / (double) stat_bytes_in);
    log_stderr ("Match8 weight in input = %zu / %zu = %lf", stat_match8_count * 8, stat_bytes_in, (double) (stat_match8_count * 8) / (double) stat_bytes_in);
    //log_stderr ("Match weight in input = %zu / %zu = %lf", stat_match_length, stat_bytes_in, (double) stat_match_length / (double) stat_bytes_in);
    log_stderr ("Average literal length = %zu / %zu = %lf", stat_literal_length, stat_literal_count, (double) stat_literal_length / (double) stat_literal_count);
//    log_stderr ("Average match length = %zu / %zu = %lf", stat_match_length, stat_match_count, (double) stat_match_length / (double) stat_match_count);
    log_stderr ("Input descrepancy = %zu", stat_bytes_in - stat_literal_length - stat_match4_count * 4 - stat_match8_count * 8);
#endif
}
