#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#endif

/** @file chain-io/read.h
   
    This file provides an interface for reading from an abstracted source into a buffer. See \ref src/chain-io/README.md for more information on the chain-io library.
 */

typedef struct chain_read chain_read; ///< A handle for read operations
typedef struct chain_read_interface chain_read_interface;
typedef chain_status (*chain_read_update_func)(chain_read_interface * interface, void * arg); ///< A read subroutine function pointer
typedef void (*chain_read_cleanup_func)(void * arg); ///< A cleanup subroutine function pointer. Note that these functions should not free the memory directly pointed to by arg, they should only clean up memory referenced within arg.
struct chain_read_interface {
    buffer_unsigned_char buffer; ///< The output buffer
    chain_read_update_func update; ///< The current update function
    chain_read_cleanup_func cleanup; ///< The current cleanup function
}; ///< The common interface for read subroutines

chain_read * chain_read_new (size_t arg_size); ///< Create a new reading handle with an arg of the given size. The contents of this arg will be initialized to zero.
void * chain_read_access_arg (chain_read * chain); ///< Returns a pointer to the arg associated with the given handle
chain_read_interface * chain_read_access_interface (chain_read * chain); ///< Returns a pointer to the interface associated with the given handle

bool chain_pull (range_const_unsigned_char * contents, chain_read * chain, size_t target_size);
/**< 
@brief Provides the next bytes to be read from the given chain. This function will pull in new bytes only if no bytes have been recently released (e.g., it was run again without a previous chain_release) or if target_size exceeds the currently cached bytes.
@param contents Will point to the next region of bytes to be processed.
@param chain The chain to be polled
@param target_size The ideal size of this read
 */
bool chain_read_is_error (chain_read * chain); ///< Returns true if an error occurred during a previous read from chain, false otherwise
void chain_release(chain_read * chain, size_t size); ///< Queues the specified number of bytes to be released from the beginning of the chain's cache. These bytes will only be freed on the next chain_pull, and can still safetly be used before then.

void chain_read_free (chain_read * chain); ///< Frees the given handle and any data associated with it.
