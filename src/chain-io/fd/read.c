#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#define FLAT_INCLUDES
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../../keyargs/keyargs.h"
#include "../common.h"
#include "../read.h"
#include "read.h"
#include "../../buffer_io/buffer_io.h"

static chain_status chain_read_fd_update_func (chain_read_interface * interface, void * arg)
{
    int fd = *(int*)arg;
    long long int size = buffer_read (.buffer = &interface->buffer.char_cast,
				      .initial_alloc_size = 1e6,
				      .fd = fd);

    if (size < 0)
    {
	return CHAIN_ERROR;
    }

    if (size == 0)
    {
	return CHAIN_COMPLETE;
    }

    return CHAIN_INCOMPLETE;
}

chain_read * chain_read_open_fd (int fd)
{
    chain_read * retval = chain_read_new(sizeof(int));

    *chain_read_access_interface(retval) = (chain_read_interface)
    {
	.update = chain_read_fd_update_func,
    };
    
    *(int*) chain_read_access_arg (retval) = fd;
    
    return retval;
}
