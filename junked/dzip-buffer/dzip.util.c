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
    dzip_deflate_state * state = dzip_deflate_new();

    buffer_unsigned_char * input_buffer = dzip_deflate_input_buffer(state);

    long long size;

    buffer_unsigned_char output = {0};

    size_t wrote_size;
    
    while (!dzip_deflate_is_done(state))
    {
	size = buffer_read (.fd = in,
			    .buffer = &input_buffer->char_cast);
	
	if (size < 0)
	{
	    log_fatal ("Error reading from input");
	}
	
	if (!size)
	{
	    dzip_deflate_eof(state);
	}

	do {
	    while (dzip_deflate_update(&output, state))
	    {}

	    wrote_size = 0;
	    
	    while (0 < (size = buffer_write (.fd = out,
					     .buffer = &output.range_cast.char_cast.const_cast,
					     .wrote_size = &wrote_size)))
	    {}
	    
	    if (size < 0)
	    {
		log_fatal ("Error writing to output");
	    }
	    
	    buffer_rewrite (output);
	    
	} while (dzip_deflate_is_full (&output, state));
		
    };

    return true;
    
fail:
    return false;
}

bool inflate_fd (int in, int out)
{
    dzip_inflate_state * state = dzip_inflate_new();

    buffer_unsigned_char * input_buffer = dzip_inflate_input_buffer(state);

    long long size;

    range_const_unsigned_char output;
    
    size_t wrote_size;

    while (!dzip_inflate_is_done(state))
    {
	size = buffer_read (.fd = in,
			    .buffer = &input_buffer->char_cast);
	
	if (size < 0)
	{
	    log_fatal ("Error reading from input");
	}
	
	if (!size)
	{
	    dzip_inflate_eof(state);
	}
	
	do {
	    while (dzip_inflate_update(state))
	    {}

	    if (dzip_inflate_error(state))
	    {
		log_fatal ("extractor error");
	    }

	    dzip_inflate_get_output(&output, state);

	    wrote_size = 0;
		
	    while (0 < (size = buffer_write (.fd = out,
					     .buffer = &output.char_cast.const_cast,
					     .wrote_size = &wrote_size)))
	    {}
		
	    if (size < 0)
	    {
		log_fatal ("Error writing to output");
	    }
		
	    dzip_inflate_release_output(state, range_count(output));

	} while (range_count (output));
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
