#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef uint32_t dzip_size;

typedef struct {
    char magic[8];
    dzip_size version;
    dzip_size chunk_size;
}
    dzip_header;

typedef struct {
    dzip_header header;
    unsigned char bytes[];
}
    dzip_chunk;

#define DZIP_MAGIC_INITIALIZER {'d', 'e', 'e', 'z', 'w', 'h', 'a', 't'}
#define DZIP_VERSION 1

// deflate

typedef struct dzip_deflate_state dzip_deflate_state;

keyargs_declare (dzip_deflate_state*, dzip_deflate_state_new);

#define dzip_deflate_state_new(...) keyargs_call (dzip_deflate_state_new, __VA_ARGS__)

keyargs_declare (void, dzip_deflate,
		 buffer_unsigned_char * output;
		 const range_const_unsigned_char * input;
		 dzip_deflate_state * state;);

#define dzip_deflate(...) keyargs_call (dzip_deflate, __VA_ARGS__)

void dzip_deflate_state_free(dzip_deflate_state * state);

// inflate

typedef struct dzip_inflate_state dzip_inflate_state;

dzip_inflate_state * dzip_inflate_state_new();

keyargs_declare (bool, dzip_inflate,
		 buffer_unsigned_char * output;
		 const dzip_chunk * chunk;
		 dzip_inflate_state * state;);

#define dzip_inflate(...) keyargs_call (dzip_inflate, __VA_ARGS__)

bool dzip_inflate_read_chunk (buffer_unsigned_char * chunk, int fd);

void dzip_print_stats();
