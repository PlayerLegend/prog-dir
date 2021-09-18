#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../vluint/vluint.h"
#endif

typedef enum metahash_id metahash_id;
enum metahash_id {
    METAHASH_IDENTITY = 0x0,
    METAHASH_SHA1 = 0x11,
    METAHASH_SHA2_256,
    METAHASH_SHA2_512,
    METAHASH_SHA3_512 = 0x14,
    METAHASH_SHA3_384,
    METAHASH_SHA3_256,
    METAHASH_SHA3_224,
    METAHASH_SHAKE_128 = 0x18,
    METAHASH_SHAKE_256,
    METAHASH_KECCAK_224 = 0x1a,
    METAHASH_KECCAK_256,
    METAHASH_KECCAK_384,
    METAHASH_MURMUR3_128 = 0x22,
    METAHASH_MURMUR3_32,
    METAHASH_MD5_128,
    METAHASH_DEFAULT = METAHASH_SHA2_256
};

range_typedef(metahash_id,metahash_id);
buffer_typedef(metahash_id,metahash_id);

metahash_id is_metahash_id (metahash_id input);
size_t metahash_size (metahash_id input);
const char * metahash_name (metahash_id input);

keyargs_declare(bool, metahash_unwrap, metahash_id * id; range_const_unsigned_char *digest, *rest; const range_const_unsigned_char *input; vluint_result * version;);
#define metahash_unwrap(...) keyargs_call(metahash_unwrap, __VA_ARGS__)

#define metahash_range metahash_range_sha256

keyargs_declare(bool, metahash_fd, const range_const_metahash_id * use_hashes; buffer_unsigned_char * output; int in_fd, out_fd;);
#define metahash_fd(...) keyargs_call(metahash_fd, __VA_ARGS__)

keyargs_declare(void, metahash_header_write, buffer_unsigned_char * output; metahash_id id;);
#define metahash_header_write(...) keyargs_call(metahash_header_write, __VA_ARGS__)

keyargs_declare(bool, metahash_header_read, const range_const_unsigned_char * input; range_const_unsigned_char * rest; metahash_id * id; vluint_result * version;);
#define metahash_header_read(...) keyargs_call(metahash_header_read, __VA_ARGS__)

metahash_id metahash_identify_name (const char * name);
bool metahash_identify_list (buffer_metahash_id * list, char * names);
