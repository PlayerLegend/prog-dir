#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../keyargs/keyargs.h"
#include "../../array/range.h"
#include "../../array/buffer.h"
#endif

/** @file dzip/deflate/deflate.h

    This file provides basic deflate functions for dzip. The intended workflow is to create a dzip_deflate_state using dzip_deflate_state_new and then use dzip_deflate or dzip_deflate_chunk to convert uncompressed bytes into compressed dzip chunks.
 */

typedef struct dzip_deflate_state dzip_deflate_state; ///< A state used by related deflate operations

keyargs_declare (dzip_deflate_state*, dzip_deflate_state_new);
#define dzip_deflate_state_new(...) keyargs_call (dzip_deflate_state_new, __VA_ARGS__) ///< Create a new deflate state

void dzip_deflate_chunk (buffer_unsigned_char * output, dzip_deflate_state * state, const range_const_unsigned_char * input); ///< Deflates all of the given input as a single chunk

void dzip_deflate (buffer_unsigned_char * output, dzip_deflate_state * state, const range_const_unsigned_char * input); ///< Deflates the input into one or more chunks of optimal size
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

void dzip_print_stats(); ///< Prints stats from previous dzip deflate operations. Only useful if DZIP_RECORD_STATS is defined.
