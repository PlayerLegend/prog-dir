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
typedef struct chain_write_interface chain_write_interface;
typedef chain_status (*chain_write_update_func)(chain_write_interface * interface, void * arg, range_const_unsigned_char * input);
typedef void (*chain_write_cleanup_func)(void * arg);
struct chain_write_interface {
    chain_write_update_func update;
    chain_write_cleanup_func cleanup;
    size_t flush_size;
};

chain_write * chain_write_new(size_t arg_size);
void * chain_write_access_arg (chain_write * chain);
chain_write_interface * chain_write_access_interface (chain_write * chain);
buffer_unsigned_char * chain_push_start (chain_write * write);
bool chain_push_final (chain_write * write);
bool chain_push (chain_write * chain, const range_const_unsigned_char * input);
bool chain_flush (chain_write * chain);
bool chain_write_is_error (chain_write * chain);
void chain_write_free (chain_write * chain);
