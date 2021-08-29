#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#endif

// deflate

typedef struct dzip_deflate_state dzip_deflate_state;
typedef uint64_t dzip_size;

keyargs_declare (dzip_deflate_state*, dzip_deflate_state_new);

#define dzip_deflate_state_new(...) keyargs_call (dzip_deflate_state_new, __VA_ARGS__);

keyargs_declare (void, dzip_deflate,
		 buffer_char * output;
		 const range_const_char * input;
		 dzip_deflate_state * state;);

#define dzip_deflate(...) keyargs_call (dzip_deflate, __VA_ARGS__);

// inflate

typedef struct dzip_inflate_state dzip_inflate_state;

dzip_inflate_state * dzip_inflate_state_new();

keyargs_declare (bool, dzip_inflate,
		 buffer_char * output;
		 const range_const_char * input;
		 dzip_inflate_state * state;);

#define dzip_inflate(...) keyargs_call (dzip_inflate, __VA_ARGS__);

void dzip_print_stats();
