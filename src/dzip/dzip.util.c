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

bool write_deflate_output (dzip_deflate_state * state)
{
    size_t wrote_size = 0;
    long int size = 0;

    static buffer_unsigned_char output = {0};

    buffer_rewrite (output);

    while (dzip_deflate_update(&output, state))
    {
	wrote_size = 0;
	
	while (0 < (size = buffer_write (.fd = STDOUT_FILENO,
					 .buffer = (range_const_char*)&output.range_cast.const_cast,
					 .wrote_size = &wrote_size)))
	{}

	if (size < 0)
	{
	    log_fatal ("Error writing to stdout");
	}
	
	buffer_rewrite (output);
    }

    return true;

fail:
    return false;
}

bool write_inflate_output (dzip_inflate_state * state)
{
    size_t wrote_size;
    long int size;

    while (dzip_inflate_update(state))
    {}
    
    range_const_unsigned_char output;

    dzip_inflate_get_output(&output, state);
    
    wrote_size = 0;
    
    while (0 < (size = buffer_write (.fd = STDOUT_FILENO,
				     .buffer = (range_const_char*) &output,
				     .wrote_size = &wrote_size)))
    {}
    
    if (size < 0)
    {
	log_fatal ("Error writing to stdout");
    }
//}

    dzip_inflate_release_output(state, range_count(output));

    if (dzip_inflate_check_error(state))
    {
	log_fatal ("dzip inflate encountered an error");
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
	//dzip_deflate_state * state = dzip_deflate_new(.window_size = 1500 / 2, .skip_count = 9);
	//dzip_deflate_state * state = dzip_deflate_new(.window_size = 1024, .skip_count = 0);
	dzip_deflate_state * state = dzip_deflate_new(.window_size = 10e6, .skip_count = 0);
	buffer_unsigned_char * input_buffer = dzip_deflate_input_buffer(state);

	long int size;

	while (0 < (size = buffer_read (.fd = STDIN_FILENO,
					.buffer = (buffer_char*)input_buffer)))
	{
	    if (!write_deflate_output(state))
	    {
		log_fatal ("Failed to write deflate output");
	    }
	}

	if (size < 0)
	{
	    log_fatal ("Error reading from stdin");
	}

	dzip_deflate_eof (state);

	if (!write_deflate_output(state))
	{
	    log_fatal ("Failed to write final deflate output");
	}

	return 0;
    }
    else
    {
	dzip_inflate_state * state = dzip_inflate_new();

	buffer_unsigned_char * input_buffer = dzip_inflate_input_buffer(state);

	long int size;

	while (0 < (size = buffer_read (.fd = STDIN_FILENO,
					.buffer = (buffer_char*)input_buffer)))
	{
	    /*if (!write_inflate_output(state))
	    {
		log_fatal ("Failed to write inflate output");
		}*/
	}

	if (size < 0)
	{
	    log_fatal ("Error reading from stdin");
	}

	dzip_inflate_eof (state);

	if (!write_inflate_output(state))
	{
	    log_fatal ("Failed to write final inflate output");
	}

	return 0;
    }

fail:
    return 1;
}
