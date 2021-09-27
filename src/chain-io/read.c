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
#include "read.h"
#include "../log/log.h"

struct chain_read {
    size_t released;
    bool is_error;
    bool is_complete;
    bool fresh_read;
    chain_read_interface interface;
    unsigned char arg[];
};

chain_read * chain_read_new (size_t arg_size)
{
    return calloc (1, sizeof (chain_read) + arg_size);
}

void * chain_read_access_arg (chain_read * chain)
{
    return chain->arg;
}

chain_read_interface * chain_read_access_interface (chain_read * chain)
{
    return &chain->interface;
}

static void chain_read_apply_release (chain_read * chain)
{
    if (!chain->released)
    {
	return;
    }
    
    size_t buffer_size = range_count (chain->interface.buffer);

    if (chain->released >= buffer_size)
    {
	buffer_rewrite (chain->interface.buffer);
	chain->released -= buffer_size;
    }
    else if (chain->released > buffer_size / 2 || buffer_size > 1e7)
    {
	buffer_downshift (chain->interface.buffer, chain->released);
	chain->released = 0;
    }
}

bool chain_pull (range_const_unsigned_char * contents, chain_read * chain, size_t target_size)
{
    chain_read_apply_release (chain);

    if (chain->is_complete)
    {
	return false;
    }

    if (chain->fresh_read)
    {
	chain->fresh_read = false;

	if (target_size + chain->released <= (size_t)range_count (chain->interface.buffer))
	{
	    goto incomplete;    
	}
    }
    
    size_t start_size = range_count (chain->interface.buffer);
    size_t max_size = start_size < target_size ? target_size : start_size + target_size;

    chain_status status = CHAIN_ERROR;

    while (true)
    {
	status = chain->interface.update (&chain->interface, chain->arg);

	if (status == CHAIN_ERROR)
	{
	    goto error;
	}
	
	if (status == CHAIN_COMPLETE)
	{
	    goto complete;
	}

	if (max_size < (size_t) range_count (chain->interface.buffer))
	{
	    goto incomplete;
	}
    }

complete:

    chain->is_complete = true;

incomplete:
    
    contents->begin = chain->interface.buffer.begin + chain->released;
    contents->end = chain->interface.buffer.end;

    if (contents->begin > contents->end)
    {
	contents->begin = contents->end;
    }

    return true;

error:
    chain->is_error = true;
    return false;
}

void chain_release(chain_read * chain, size_t size)
{
    chain->released += size;

    if (size)
    {
	chain->fresh_read = true;
    }
}

void chain_read_free (chain_read * chain)
{
    free (chain->interface.buffer.begin);
    if (chain->interface.cleanup)
    {
	chain->interface.cleanup (chain->arg);
    }
    free (chain);
}

bool chain_read_is_error (chain_read * chain)
{
    return chain->is_error;
}
