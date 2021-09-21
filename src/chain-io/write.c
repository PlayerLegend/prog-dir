#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#include "write.h"
#include "../libc/string.h"
#include "../log/log.h"

struct chain_write {
    chain_write_update_func func;
    void * arg;
    size_t written;
    void (*free_arg)(void * arg);
    buffer_unsigned_char buffer;
    size_t flush_size;
};

#define BUFFER_MAX_SIZE 1e7
#define INPUT_FLUSH_SIZE 1e6

keyargs_define(chain_write_new)
{
    chain_write * chain = calloc (1, sizeof(*chain));

    chain->func = args.func;
    chain->arg = args.arg;
    chain->free_arg = args.free_arg;
    chain->flush_size = args.flush_size;

    return chain;
}

static bool chain_write_flush_input (chain_write * chain, range_const_unsigned_char * input)
{
    chain_status status = CHAIN_ERROR;
    
    while (true)
    {
	status = chain->func (input, chain->arg);

	if (status != CHAIN_INCOMPLETE)
	{
	    if (status == CHAIN_ERROR)
	    {
		return false;
	    }
	    else
	    {
		assert (status == CHAIN_COMPLETE);
		return true;
	    }
	}
    }
}

bool chain_write_update (chain_write * chain, const range_const_unsigned_char * input)
{
    if (range_is_empty (*input))
    {
	return true;
    }
    
    if (chain->written)
    {
	size_t buffer_size = range_count (chain->buffer);
	
	if (chain->written > buffer_size / 2 || buffer_size > BUFFER_MAX_SIZE)
	{
	    buffer_downshift (chain->buffer, chain->written);
	    chain->written = 0;
	}
    }

    assert ((size_t)range_count (chain->buffer) >= chain->written);
    
    if ((size_t)range_count (chain->buffer) == chain->written && (size_t)range_count (*input) > chain->flush_size)
    {
	range_const_unsigned_char input_modify = *input;
	
	if (chain_write_flush_input(chain, &input_modify))
	{
	    assert (chain->written == (size_t)range_count (chain->buffer));
	    buffer_append (chain->buffer, input_modify);
	    return true;
	}
	else
	{
	    return false;
	}
    }
    else
    {
	buffer_append (chain->buffer, *input);
	
	if (range_count(chain->buffer) - chain->written < INPUT_FLUSH_SIZE)
	{
	    return true;
	}
	else
	{
	    return chain_write_flush (chain);
	}
    }
}

bool chain_write_flush(chain_write * chain)
{
    range_const_unsigned_char input =
	{ .begin = chain->buffer.range_cast.const_cast.begin + chain->written,
	  .end = chain->buffer.range_cast.const_cast.end, };

    if (chain_write_flush_input(chain, &input))
    {
	chain->written = input.begin - chain->buffer.begin;
	return true;
    }
    else
    {
	return false;
    }
}


void chain_write_free (chain_write * chain)
{
    chain_write_flush (chain);
    free (chain->buffer.begin);
    if (chain->free_arg)
    {
	chain->free_arg (chain->arg);
    }
    free (chain);
}
