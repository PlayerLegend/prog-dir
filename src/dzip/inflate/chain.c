#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../keyargs/keyargs.h"
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../../chain-io/common.h"
#include "../../chain-io/read.h"
#include "../../chain-io/write.h"
#include "../common/common.h"
#include "inflate.h"
#include "extensions.h"
#include "chain.h"

#include "../../log/log.h"

static chain_status input_update_func (chain_read_interface * interface, void * arg_orig)
{
    chain_read * input = *(chain_read**)arg_orig;
    range_const_unsigned_char contents;
    
    if (!chain_pull (&contents, input, 1e6))
    {
	return chain_read_is_error(input) ? CHAIN_ERROR : CHAIN_COMPLETE;
    }

    const unsigned char * release_begin = contents.begin;
    
    if (!dzip_inflate_range (&interface->buffer, &contents))
    {
	return CHAIN_ERROR;
    }

    chain_release (input, contents.begin - release_begin);

    return CHAIN_INCOMPLETE;
}

chain_read * dzip_inflate_chain_input (chain_read * input)
{
    chain_read * retval = chain_read_new (sizeof(chain_read*));

    chain_read_access_interface(retval)->update = input_update_func;
    *(chain_read**) chain_read_access_arg (retval) = input;
	
    return retval;
}

static chain_status output_update_func (chain_write_interface * interface, void * arg_orig, range_const_unsigned_char * compressed_contents)
{
    chain_write * chain = *(chain_write**) arg_orig;

    const unsigned char * inflate_start = compressed_contents->begin;
    
    dzip_inflate_range(chain_push_start(chain), compressed_contents);

    if (!chain_push_final(chain))
    {
	return CHAIN_ERROR;
    }

    if (compressed_contents->begin == inflate_start)
    {
	return CHAIN_COMPLETE;
    }
    else
    {
	return CHAIN_INCOMPLETE;
    }
}

chain_write * dzip_inflate_chain_output (chain_write * input)
{
    chain_write * retval = chain_write_new (sizeof(chain_write*));

    chain_write_access_interface (retval)->update = output_update_func;
    
    *(chain_write**) chain_write_access_arg (retval) = input;

    return retval;
}
