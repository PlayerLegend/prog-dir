#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#endif

typedef enum {
    CHAIN_ERROR,
    CHAIN_INCOMPLETE,
    CHAIN_COMPLETE
}
    chain_status;
/*
typedef struct chain_io chain_io;

typedef void (*chain_read)(buffer_unsigned_char * output, void * arg);
typedef void (*chain_write)(void * arg, range_const_unsigned_char * available);

keyargs_declare(chain_io*, chain_new,
		chain_read read;
		void * read_arg;
		void (*free_read_arg)(void * arg);
		chain_write write;
		void * write_arg;
		void (*free_write_arg)(void * arg););
#define chain_read_new(...) keyargs_call(chain_read_new, __VA_ARGS__)

bool chain_push (chain_io * chain, const range_const_unsigned_char * input);
bool chain_pull (buffer_unsigned_char * output, chain_io * chain);
void chain_release (chain_io * chain, size_t size);
bool chain_is_read_error (chain_io * chain);
bool chain_is_read_complete (chain_io * chain);
bool chain_is_write_error (chain_io * chain);
bool chain_is_write_complete (chain_io * chain);
*/
