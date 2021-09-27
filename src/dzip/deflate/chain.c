#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../keyargs/keyargs.h"
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../../chain-io/common.h"
#include "../../chain-io/read.h"
#include "../../chain-io/write.h"
#include "deflate.h"
#include "chain.h"

#include "../../log/log.h"

//    input

typedef struct {
    chain_read * input;
    dzip_deflate_state * state;
}
    input_arg;

static chain_status input_update_func (chain_read_interface * interface, void * arg_orig)
{
    input_arg * arg = arg_orig;
    range_const_unsigned_char contents = {0};

    if (!chain_pull (&contents, arg->input, 1e6))
    {
	return chain_read_is_error(arg->input) ? CHAIN_ERROR : CHAIN_COMPLETE;
    }

    dzip_deflate (&interface->buffer, arg->state, &contents);

    chain_release (arg->input, range_count (contents));

    return CHAIN_INCOMPLETE;
}

static void input_arg_cleanup (void * arg_orig)
{
    input_arg * arg = arg_orig;
    dzip_deflate_state_free(arg->state);
}

chain_read * dzip_deflate_chain_input (chain_read * input)
{
    chain_read * read = chain_read_new (sizeof(input_arg));

    *chain_read_access_interface (read) = (chain_read_interface)
    {
	.update = input_update_func,
	.cleanup = input_arg_cleanup,
    };
    
    *(input_arg*) chain_read_access_arg (read) = (input_arg)
    {
	.input = input,
	.state = dzip_deflate_state_new(),
    };

    return read;
}

//    output

typedef struct {
    chain_write * output;
    dzip_deflate_state * state;
}
    output_arg;

static chain_status output_update_func (chain_write_interface * interface, void * arg_orig, range_const_unsigned_char * input)
{
    output_arg * arg = arg_orig;

    buffer_unsigned_char * access = chain_push_start(arg->output);
    
    dzip_deflate (access, arg->state, input);

    return chain_push_final(arg->output) ? CHAIN_COMPLETE : CHAIN_ERROR;
}

static void output_arg_cleanup (void * arg_orig)
{
    output_arg * arg = arg_orig;
    dzip_deflate_state_free(arg->state);
}

chain_write * dzip_deflate_chain_output (chain_write * output)
{
    chain_write * retval = chain_write_new (sizeof(output_arg));

    *chain_write_access_interface (retval) = (chain_write_interface)
    {
	.update = output_update_func,
	.cleanup = output_arg_cleanup,
    };
    
    *(output_arg*) chain_write_access_arg (retval) = (output_arg)
    {
	.output = output,
	.state = dzip_deflate_state_new(),
    };

    return retval;
}
