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

static chain_status chain_write_fd_update_func(chain_write_interface * interface, void * arg, range_const_unsigned_char * input)
{
    int fd = *(int*) arg;
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
    chain_write * retval = chain_write_new (sizeof(int));

    *chain_write_access_interface (retval) = (chain_write_interface)
	{
	    .update = chain_write_fd_update_func,
	    .flush_size = 1e6,
	};
    
    *(int*) chain_write_access_arg (retval) = fd;

    return retval;
}
