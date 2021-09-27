#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../../keyargs/keyargs.h"
#include "../common.h"
#include "../write.h"
#endif

/** @file chain-io/fd/write.h

    This file provides a chain_write overlay for file descriptor writing. Definitions for STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO are also created if they do not already exist.
 
*/

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

chain_write * chain_write_open_fd (int fd); ///< Creates a chain_write handle for writing to the given file descriptor
