#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#define FLAT_INCLUDES
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../../keyargs/keyargs.h"
#include "../../chain-io/common.h"
#include "../../chain-io/read.h"
#include "../../chain-io/write.h"
#include "../../chain-io/fd/read.h"
#include "../../chain-io/fd/write.h"
#include "../deflate/deflate.h"
#include "../deflate/chain.h"
#include "../inflate/chain.h"
#include "../common/common.h"
#include "../inflate/inflate.h"
#include "../inflate/extensions.h"
#include "../../buffer_io/buffer_io.h"
#include "../../log/log.h"

#if 1

int main(int argc, char * argv[])
{
    bool should_deflate = true;
    
    for (int i = 0; i < argc; i++)
    {
	if (0 == strcmp (argv[i], "-d"))
	{
	    should_deflate = false;
	}
    }

    chain_read * fd_in = chain_read_open_fd(STDIN_FILENO);
    chain_write * fd_out = chain_write_open_fd(STDOUT_FILENO);
    
    chain_read * dzip_in = (should_deflate ? dzip_deflate_chain_input : dzip_inflate_chain_input)(fd_in);
    range_const_unsigned_char contents;

    while (chain_pull(&contents, dzip_in, 1e6))
    {
	//log_debug ("now pushing");
	chain_push (fd_out, &contents);
	chain_release (dzip_in, range_count(contents));
    }
    
    if (chain_read_is_error(dzip_in))
    {
	log_fatal ("Read error");
    }
    
    if (chain_write_is_error(fd_out))
    {
	log_fatal ("Write error");
    }

    chain_read_free(dzip_in);
    chain_read_free(fd_in);
    chain_write_free(fd_out);

    return 0;

fail:
    return 1;
}
#else
bool deflate_fd (int in, int out)
{
    dzip_deflate_state * state = dzip_deflate_state_new();

    long int size;

    buffer_unsigned_char input = {0};
    buffer_unsigned_char output = {0};

    size_t wrote_size;

    while (true)
    {
	buffer_rewrite (input);
	buffer_rewrite (output);
	
	while (0 < (size = buffer_read (.fd = in,
					.buffer = &input.char_cast,
					.initial_alloc_size = 1e6,
					.max_buffer_size = 1e6)))
	{}

	if (size < 0)
	{
	    log_fatal ("failed while reading input");
	}

	if (range_is_empty (input))
	{
	    return true;
	}

	dzip_deflate (&output, state, &input.range_cast.const_cast);

	wrote_size = 0;

	while (0 < (size = buffer_write (.fd = out,
					 .buffer = &output.char_cast.range_cast.const_cast,
					 .wrote_size = &wrote_size)))
	{}

	if (size < 0)
	{
	    log_fatal ("failed while writing output");
	}
    }

    assert (false);
    
fail:
    return false;
}

bool inflate_fd (int in, int out)
{
    long int size;

    buffer_unsigned_char input = {0};
    buffer_unsigned_char output = {0};

    size_t wrote_size;

    while (true)
    {
	if (!dzip_inflate_read_chunk(&input, in))
	{
	    log_fatal ("Failed to read a chunk");
	}

	if (range_is_empty (input))
	{
	    return true;
	}

	buffer_rewrite (output);

	if (!dzip_inflate (.chunk = (dzip_chunk*) input.begin, .output = &output))
	{
	    log_fatal ("Failed while inflating");
	}

	wrote_size = 0;

	while (0 < (size = buffer_write (.fd = out,
					 .buffer = &output.char_cast.range_cast.const_cast,
					 .wrote_size = &wrote_size)))
	{}

	if (size < 0)
	{
	    log_fatal ("failed while writing output");
	}
    }

    return true;
    
fail:
    return false;
}

int main(int argc, char * argv[])
{
    bool deflate = true;
    
    for (int i = 0; i < argc; i++)
    {
	if (0 == strcmp (argv[i], "-d"))
	{
	    deflate = false;
	}
    }

    if (deflate)
    {
	return deflate_fd (STDIN_FILENO, STDOUT_FILENO) ? 0 : 1;
    }
    else
    {
	return inflate_fd (STDIN_FILENO, STDOUT_FILENO) ? 0 : 1;
    }
}
#endif
