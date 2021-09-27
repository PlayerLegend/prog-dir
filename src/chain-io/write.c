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
    size_t written;
    buffer_unsigned_char buffer;
    chain_write_interface interface;
    bool is_error;
    unsigned char arg[];
};

#define BUFFER_MAX_SIZE 1e7
#define INPUT_FLUSH_SIZE 1e6

chain_write * chain_write_new(size_t arg_size)
{
    return calloc (1, sizeof(chain_write) + arg_size);
}

chain_write_interface * chain_write_access_interface (chain_write * chain)
{
    return &chain->interface;
}

void * chain_write_access_arg (chain_write * chain)
{
    return chain->arg;
}

static bool chain_write_flush_input (chain_write * chain, range_const_unsigned_char * input)
{
    if (chain->is_error)
    {
	return false;
    }
    
    chain_status status = CHAIN_ERROR;
    
    while (true)
    {
	status = chain->interface.update (&chain->interface, chain->arg, input);

	if (status == CHAIN_INCOMPLETE)
	{
	    continue;
	}
	else if (status == CHAIN_COMPLETE)
	{
	    return true;
	}
	else
	{
	    assert (status == CHAIN_ERROR);
	    chain->is_error = true;
	    return false;
	}
    }
}

bool chain_push (chain_write * chain, const range_const_unsigned_char * input)
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
    
    if ((size_t)range_count (chain->buffer) == chain->written && (size_t)range_count (*input) > chain->interface.flush_size)
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

	return chain_push_final (chain);
    }
}

buffer_unsigned_char * chain_push_start (chain_write * chain)
{
    return &chain->buffer;
}

bool chain_push_final (chain_write * chain)
{
    if (range_count(chain->buffer) - chain->written < INPUT_FLUSH_SIZE)
    {
	return true;
    }
    else
    {
	return chain_flush (chain);
    }
}

bool chain_flush (chain_write * chain)
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
    chain_flush (chain);
    free (chain->buffer.begin);
    if (chain->interface.cleanup)
    {
	chain->interface.cleanup (chain->arg);
    }
    free (chain);
}

bool chain_write_is_error (chain_write * chain)
{
    return chain->is_error;
}
