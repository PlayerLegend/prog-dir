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
typedef struct chain_read_interface chain_read_interface;
typedef chain_status (*chain_read_update_func)(chain_read_interface * interface, void * arg);
typedef void (*chain_read_cleanup_func)(void * arg);
struct chain_read_interface {
    buffer_unsigned_char buffer;
    chain_read_update_func update;
    chain_read_cleanup_func cleanup;
};

chain_read * chain_read_new (size_t arg_size);
void * chain_read_access_arg (chain_read * chain);
chain_read_interface * chain_read_access_interface (chain_read * chain);

bool chain_pull (range_const_unsigned_char * contents, chain_read * chain, size_t target_size);
bool chain_read_is_error (chain_read * chain);
void chain_release(chain_read * chain, size_t size);

void chain_read_free (chain_read * chain);
