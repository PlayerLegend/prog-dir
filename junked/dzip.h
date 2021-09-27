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
