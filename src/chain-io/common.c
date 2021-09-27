#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "common.h"

struct chain_io {
    buffer_unsigned_char input_buffer;
    buffer_unsigned_char output_buffer;
    size_t released;
    bool write_error;
    bool write_complete;
    bool read_error;
    bool read_complete;
};

bool chain_push (chain_io * chain, const range_const_unsigned_char * input)
{
    
}

bool chain_pull (buffer_unsigned_char * output, chain_io * chain);
bool chain_is_error (chain_io * chain);
bool chain_is_complete (chain_io * chain);
void chain_release (chain_io * chain, size_t size);
