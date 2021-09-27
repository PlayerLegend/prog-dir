#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#endif

/** @file chain-io/common.h
    
    This file provides common definitions for the chain-io library. For a detailed explanation of this library, see \ref src/chain-io/README.md.
*/

typedef enum {
    CHAIN_ERROR, ///< Indicates that an error occurred
    CHAIN_INCOMPLETE, ///< Indicates that more bytes may be available
    CHAIN_COMPLETE ///< Indicates that no more bytes will be available
}
    chain_status; ///< A status indicator used by both the chain_read and chain_write interfaces
