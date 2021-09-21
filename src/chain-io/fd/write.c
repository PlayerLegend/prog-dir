#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#define FLAT_INCLUDES
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../../keyargs/keyargs.h"
#include "../common.h"
#include "../write.h"
#include "write.h"
#include "../../buffer_io/buffer_io.h"

static chain_status chain_write_fd_update_func(range_const_unsigned_char * input, void * arg)
{
    int fd = (uintptr_t) arg;
    size_t wrote_size = 0;
    long long int size = buffer_write (.fd = fd,
				       .buffer = &input->char_cast.const_cast,
				       .wrote_size = &wrote_size);

    if (size < 0)
    {
	return CHAIN_ERROR;
    }

    assert (wrote_size == (size_t)size);

    if (size == 0)
    {
	return CHAIN_COMPLETE;
    }
    
    input->begin += wrote_size;

    return CHAIN_INCOMPLETE;
}

chain_write * chain_write_open_fd (int fd)
{
    uintptr_t arg_intptr = fd;
    void * arg = (void*) arg_intptr;
    
    return chain_write_new (.func = chain_write_fd_update_func,
			    .arg = arg,
			    .flush_size = 1e6);
}
