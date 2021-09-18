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

static io_status io_write_fd_update_func(range_const_unsigned_char * input, void * arg)
{
    int fd = (uintptr_t) arg;
    size_t wrote_size = 0;
    long long int size = buffer_write (.fd = fd,
				       .wrote_size = &wrote_size);

    if (size < 0)
    {
	return IO_ERROR;
    }

    assert (wrote_size == (size_t)size);

    if (size == 0)
    {
	return IO_COMPLETE;
    }
    
    input->begin += wrote_size;

    return IO_INCOMPLETE;
}

io_write * io_write_open_fd (int fd)
{
    uintptr_t arg_intptr = fd;
    void * arg = (void*) arg_intptr;
    
    return io_write_new (.func = io_write_fd_update_func,
			 .arg = arg,
			 .flush_size = 1e6);
}
