#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#endif

#define DZIP_MAGIC *((uint64_t*)(const char[]) { 'd', 'e', 'e', 'z', 'w', 'h', 'a', 't' })

#define DZIP_DEFAULT_WINDOW_SIZE (1 << 15)

typedef uint64_t dzip_size;

// deflate

typedef struct dzip_deflate_state dzip_deflate_state;

keyargs_declare(dzip_deflate_state*, dzip_deflate_new,
		dzip_size window_size;
		dzip_size max_match_length;
		dzip_size skip_count;);

#define dzip_deflate_new(...) keyargs_call(dzip_deflate_new, __VA_ARGS__)

buffer_unsigned_char * dzip_deflate_input_buffer (dzip_deflate_state * state);
bool dzip_deflate_update (buffer_unsigned_char * output, dzip_deflate_state * state);
void dzip_deflate_eof (dzip_deflate_state * state);
void dzip_deflate_print_stats (dzip_deflate_state * state);
void dzip_deflate_free (dzip_deflate_state * state);

// inflate

typedef struct dzip_inflate_state dzip_inflate_state;

keyargs_declare(dzip_inflate_state*, dzip_inflate_new,
		dzip_size max_chunk_size;);

#define dzip_inflate_new(...) keyargs_call(dzip_inflate_new, __VA_ARGS__);

buffer_unsigned_char * dzip_inflate_input_buffer (dzip_inflate_state * state);
bool dzip_inflate_update (dzip_inflate_state * state);
void dzip_inflate_eof (dzip_inflate_state * state);
#define dzip_inflate_check_error(inflate_p) (*(bool*)(inflate_p))
void dzip_inflate_release_output (dzip_inflate_state * state, dzip_size size);
void dzip_inflate_get_output (range_const_unsigned_char * output, dzip_inflate_state * state);
