#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#endif

/** @file chain-io/write.h
   
    This file provides an interface for writing to an abstracted destination from a buffer. See \ref src/chain-io/README.md for more information on the chain-io library.
 */
typedef struct chain_write chain_write; ///< A handle for write operations
typedef struct chain_write_interface chain_write_interface;
typedef chain_status (*chain_write_update_func)(chain_write_interface * interface, void * arg, range_const_unsigned_char * input); ///< A write subroutine function pointer
typedef void (*chain_write_cleanup_func)(void * arg); ///< A cleanup subroutine function pointer. Note that these functions should not free the memory directly pointed to by arg, they should only clean up memory referenced within arg.
struct chain_write_interface {
    chain_write_update_func update; ///< The current update function
    chain_write_cleanup_func cleanup; ///< The current cleanup function
    size_t flush_size; ///< The current size at which the buffer should be flushed through the update function
}; ///< The common interface for write subroutines

chain_write * chain_write_new(size_t arg_size); ///< Creates a new write handle with an arg of the given size. The contents of this arg will be initialized to zero.
void * chain_write_access_arg (chain_write * chain); ///< Returns a pointer to the arg associated with the given handle.
chain_write_interface * chain_write_access_interface (chain_write * chain); ///< Returns a pointer to the interface associated with a given handle.
buffer_unsigned_char * chain_push_start (chain_write * write); ///< Starts a push by returning the buffer to which bytes should be written.
bool chain_push_final (chain_write * write); ///< Finishes a push initiated by chain_push_start
bool chain_push (chain_write * chain, const range_const_unsigned_char * input); ///< Pushes data provided in a range
bool chain_flush (chain_write * chain); ///< Flushes any cached bytes in the given chain
bool chain_write_is_error (chain_write * chain); ///< Returns true if an error occurred during a previous write command, false otherwise
void chain_write_free (chain_write * chain); ///< Frees the given write handle and any associated data
