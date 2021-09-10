#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../dzip/dzip.h"
#include "../buffer_io/buffer_io.h"
#include "../log/log.h"

bool deflate_fd (int in, int out)
{
    dzip_deflate_state * state = dzip_deflate_state_new();

    long int size;

    buffer_char input = {0};
    buffer_char output = {0};

    size_t wrote_size;

    while (true)
    {
	buffer_rewrite (input);
	buffer_rewrite (output);
	
	while (0 < (size = buffer_read (.fd = in,
					.buffer = &input,
					.initial_alloc_size = 10e6,
					.max_buffer_size = 10e6)))
	{}

	if (size < 0)
	{
	    log_fatal ("failed while reading input");
	}

	if (range_is_empty (input))
	{
	    return true;
	}

	dzip_deflate (.state = state, .input = &input.range_cast.const_cast, .output = &output);

	wrote_size = 0;

	while (0 < (size = buffer_write (.fd = out,
					 .buffer = &output.range_cast.const_cast,
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
    dzip_inflate_state * state = dzip_inflate_state_new();

    long int size;

    buffer_char input = {0};
    buffer_char output = {0};

    size_t wrote_size;

    while (true)
    {
	buffer_rewrite (input);
	buffer_rewrite (output);
	
	while (0 < (size = buffer_read (.fd = in,
					.buffer = &input,
					.initial_alloc_size = 10e6,
					.max_buffer_size = 10e6)))
	{}

	if (size < 0)
	{
	    log_fatal ("failed while reading input");
	}

	if (range_is_empty (input))
	{
	    return true;
	}

	if (!dzip_inflate (.state = state, .input = &input.range_cast.const_cast, .output = &output))
	{
	    log_fatal ("Failed while inflating");
	}

	wrote_size = 0;

	while (0 < (size = buffer_write (.fd = out,
					 .buffer = &output.range_cast.const_cast,
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
