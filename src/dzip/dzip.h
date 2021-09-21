#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../chain-io/common.h"
#include "../chain-io/read.h"
#include "../chain-io/write.h"
#endif

/** @file dzip.h
 This file describes the public interface for dzip.

 The workflow for deflating a range of bytes into dzip chunks is as follows: First, allocate a dzip state with dzip_deflate_state_new, then pass this state along with your input bytes and output buffer to dzip_deflate. Your output buffer will then have one or more new deflated chunks appended to it.

 To inflate a chunk back into its original bytes, simply pass the chunk and an output buffer to dzip_inflate.

 Some quality of life functions are defined separately in extensions.c, their declarations can be found at the end of this file.
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

void dzip_print_stats(); ///< Prints stats from previous dzip deflate operations. Only useful if DZIP_RECORD_STATS is defined.

//
//    deflate
//

typedef struct dzip_deflate_state dzip_deflate_state; ///< A state used by related deflate operations

keyargs_declare (dzip_deflate_state*, dzip_deflate_state_new);
#define dzip_deflate_state_new(...) keyargs_call (dzip_deflate_state_new, __VA_ARGS__) ///< Create a new deflate state

void dzip_deflate_chunk (buffer_unsigned_char * output, dzip_deflate_state * state, const range_const_unsigned_char * input);

void dzip_deflate (buffer_unsigned_char * output, dzip_deflate_state * state, const range_const_unsigned_char * input);
/**<
   @brief Create one or more dzip chunks from input
   @param output The buffer in which to store dzip chunk(s) created from the input.
   @param input The input bytes to be deflated
   @param state The state to be used for this operation
*/

void dzip_deflate_state_free(dzip_deflate_state * state);
/**< 
   @brief Free a deflate state
    @param state The state to free
 */

chain_read * dzip_deflate_chain_input (chain_read * input);
chain_read * dzip_inflate_chain_input (chain_read * input);

chain_write * dzip_deflate_chain_output (chain_write * input);
chain_write * dzip_inflate_chain_output (chain_write * input);

//
//    inflate
//

keyargs_declare (bool, dzip_inflate,
		 buffer_unsigned_char * output;
		 const dzip_chunk * chunk;);
#define dzip_inflate(...) keyargs_call (dzip_inflate, __VA_ARGS__)
/**< 
   @brief Inflates a dzip chunk back into the original data.
   @return True if successful, false otherwise
   @param output The buffer in which to store the output data
   @param chunk The chunk to be inflated
   @param state The state to use for this operation
*/

//
//    extensions
//

bool dzip_inflate_read_chunk (buffer_unsigned_char * chunk, int fd);
/**< 
   @brief Reads a chunk from the given file descriptor
   @return True if successful, false otherwise
   @param chunk The buffer in which to store the chunk. This buffer is rewritten before the chunk is read into it.
   @param fd The file descriptor to read from
*/

long long int dzip_inflate_range (buffer_unsigned_char * output, range_const_unsigned_char * input);
/**< 
   @brief Inflates any complete chunks within the given input and returns the number of bytes processed, or -1 if an error ocurred. The intended usage for this function is to direct it at an input buffer for a dzip stream, store the number of bytes it returns, and then shift that number of bytes out of the beginning of input buffer.
   @return The number of bytes processed, or -1 if an error occurred.
   @param output The output buffer, where decompressed data will be written.
   @param input The input buffer, containing dzip chunks
 */
