#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#endif

typedef struct chain_read chain_read;
typedef chain_status (*chain_read_update_func)(buffer_unsigned_char * buffer, void * arg);

keyargs_declare(chain_read*, chain_read_new,
		chain_read_update_func func;
		void * arg;
		void (*free_arg)(void * arg););
#define chain_read_new(...) keyargs_call(chain_read_new, __VA_ARGS__)

chain_status chain_read_update (chain_read * chain);

void chain_read_contents (range_const_unsigned_char * contents, chain_read * chain);

void chain_read_release(chain_read * chain, size_t size);

void chain_read_free (chain_read * chain);
