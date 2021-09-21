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

static chain_status chain_read_fd_update_func (buffer_unsigned_char * buffer, void * arg)
{
    int fd = (uintptr_t)arg;
    long long int size = buffer_read (.buffer = &buffer->char_cast,
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
    uintptr_t arg_intptr = fd;
    void * arg = (void*) arg_intptr;
    return chain_read_new(.func = chain_read_fd_update_func,
			  .arg = arg);
}
