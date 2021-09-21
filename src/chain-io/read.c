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
    chain_read_update_func func;
    void * arg;
    size_t released;
    void (*free_arg)(void * arg);
    buffer_unsigned_char buffer;
};

#define MAX_BUFFER_SIZE ((size_t)1e7)

keyargs_define(chain_read_new)
{
    chain_read * chain = calloc (1, sizeof(*chain));

    chain->func = args.func;
    chain->arg = args.arg;
    chain->free_arg = args.free_arg;

    return chain;
}

static void chain_read_apply_release (chain_read * chain)
{
    size_t buffer_size = range_count (chain->buffer);

    if (!chain->released)
    {
	return;
    }

    if (chain->released >= buffer_size)
    {
	buffer_rewrite (chain->buffer);
	chain->released = 0;
    }
    else if (chain->released > buffer_size / 2 || buffer_size > MAX_BUFFER_SIZE)
    {
	buffer_downshift (chain->buffer, chain->released);
	chain->released = 0;
    }
}

chain_status chain_read_update (chain_read * chain)
{
    chain_read_apply_release (chain);
    
    size_t start_size = range_count (chain->buffer);
    size_t max_size = start_size < MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : start_size + MAX_BUFFER_SIZE;

    chain_status status = CHAIN_ERROR;
    
    while (true)
    {
	status = chain->func (&chain->buffer, chain->arg);
	
	if (status != CHAIN_INCOMPLETE)
	{
	    return status;
	}

	if (max_size < (size_t) range_count (chain->buffer))
	{
	    return CHAIN_INCOMPLETE;
	}
    }
}

void chain_read_contents (range_const_unsigned_char * contents, chain_read * chain)
{
    contents->begin = chain->buffer.begin + chain->released;
    contents->end = chain->buffer.end;

    if (contents->begin > contents->end)
    {
	contents->begin = contents->end;
    }
}

void chain_read_release(chain_read * chain, size_t size)
{
    chain->released += size;
}

void chain_read_free (chain_read * chain)
{
    free (chain->buffer.begin);
    if (chain->free_arg)
    {
	chain->free_arg (chain->arg);
    }
    free (chain);
}
