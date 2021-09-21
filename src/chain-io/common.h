#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef enum {
    CHAIN_ERROR,
    CHAIN_INCOMPLETE,
    CHAIN_COMPLETE
}
    chain_status;

typedef struct chain_io chain_io;
bool chain_push (chain_io * chain, const range_const_unsigned_char * input);
bool chain_pull (buffer_unsigned_char * output, chain_io * chain);
void chain_release (chain_io * chain, size_t size);
