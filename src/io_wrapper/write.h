#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#endif

typedef struct io_write io_write;
typedef io_status (*io_write_update_func)(range_const_unsigned_char * input, void * arg);

keyargs_declare(io_write*, io_write_new,
		io_write_update_func func;
		void * arg;
		void (*free_arg)(void * arg);
		size_t flush_size;);
#define io_write_new(...) keyargs_call(io_write_new, __VA_ARGS__)

bool io_write_update (io_write * io, const range_const_unsigned_char * input);
bool io_write_flush(io_write * io);
void io_write_free (io_write * io);
