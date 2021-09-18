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

static io_status io_read_fd_update_func (buffer_unsigned_char * buffer, void * arg)
{
    int fd = (uintptr_t)arg;
    long long int size = buffer_read (.buffer = &buffer->char_cast,
				      .fd = fd);

    if (size < 0)
    {
	return IO_ERROR;
    }

    if (size == 0)
    {
	return IO_COMPLETE;
    }

    return IO_INCOMPLETE;
}

io_read * io_read_open_fd (int fd)
{
    uintptr_t arg_intptr = fd;
    void * arg = (void*) arg_intptr;
    return io_read_new(.func = io_read_fd_update_func,
			       .arg = arg);
}
