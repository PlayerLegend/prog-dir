#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#endif

typedef struct io_read io_read;
typedef io_status (*io_read_update_func)(buffer_unsigned_char * buffer, void * arg);

keyargs_declare(io_read*, io_read_new,
		io_read_update_func func;
		void * arg;
		void (*free_arg)(void * arg););
#define io_read_new(...) keyargs_call(io_read_new, __VA_ARGS__)

io_status io_read_update (io_read * io);

void io_read_contents (range_const_unsigned_char * contents, io_read * io);

void io_read_release(io_read * io, size_t size);

void io_read_free (io_read * io);
