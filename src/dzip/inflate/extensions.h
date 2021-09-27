#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../keyargs/keyargs.h"
#include "../../array/range.h"
#include "../../array/buffer.h"
#endif

bool dzip_inflate_read_chunk (buffer_unsigned_char * chunk, int fd);
/**< 
   @brief Reads a chunk from the given file descriptor
   @return True if successful, false otherwise
   @param chunk The buffer in which to store the chunk. This buffer is rewritten before the chunk is read into it.
   @param fd The file descriptor to read from
*/

bool dzip_inflate_range (buffer_unsigned_char * output, range_const_unsigned_char * input);
/**< 
   @brief Inflates any complete chunks within the given input and advances the input beginning beyond the deflated chunks.
   @return True on success, false otherwise
   @param output The output buffer where decompressed data will be written.
   @param input The input range containing dzip chunks, the begin pointer of this range will be advanced past chunks as they are read.
 */
