#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#endif

typedef struct chain_write chain_write;
typedef chain_status (*chain_write_update_func)(range_const_unsigned_char * input, void * arg);

keyargs_declare(chain_write*, chain_write_new,
		chain_write_update_func func;
		void * arg;
		void (*free_arg)(void * arg);
		size_t flush_size;);
#define chain_write_new(...) keyargs_call(chain_write_new, __VA_ARGS__)

bool chain_write_update (chain_write * chain, const range_const_unsigned_char * input);
bool chain_write_flush(chain_write * chain);
void chain_write_free (chain_write * chain);
