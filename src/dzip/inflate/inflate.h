#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../keyargs/keyargs.h"
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../common/common.h"
#endif

/** @file dzip/inflate/inflate.h
    
    This file provides a basic inflate function for dzip. Unlike the deflate portion of dzip, inflation does not require a state. Simply pass a chunk of a dzip stream to dzip_inflate in order to extract it. To determine the bounds of a chunk within an input buffer, cast the contents of your buffer as a dzip_header or dzip_chunk, as defined in dzip/common/common.h. Alternatively, a wrapper for this procedure can be found in dzip/inflate/extensions.h, along with other quality of life features.
 */

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
