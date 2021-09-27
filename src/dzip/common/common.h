#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../keyargs/keyargs.h"
#include "../../array/range.h"
#include "../../array/buffer.h"
#endif

/** @file dzip/common/common.h
    This file provides common public definitions used by dzip.
 */

/// A general purpose size that is large enough to be used within dzip chunks
typedef uint32_t dzip_size;

#define DZIP_MAGIC_INITIALIZER {'d', 'e', 'e', 'z', 'w', 'h', 'a', 't'}
#define DZIP_VERSION 1

typedef struct {
    char magic[8]; ///< This should be an 8 character array containing "deezwhat" with no null terminator. Use the initializer DZIP_MAGIC_INITIALIZER to initialize an array to compare the magic value with.
    dzip_size version; ///< This is an integer which should match DZIP_VERSION
    dzip_size chunk_size; ///< This is the total size of the chunk, that is, the combined size of the header and data.
}
    dzip_header; ///< A header for a dzip stream chunk

typedef struct {
    dzip_header header; ///< The chunk header
    unsigned char bytes[]; ///< The chunk data
}
    dzip_chunk; ///< A chunk of a dzip stream. Individual chunks are independent from each other.
