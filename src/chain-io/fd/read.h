#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../../array/range.h"
#include "../../array/buffer.h"
#include "../../keyargs/keyargs.h"
#include "../common.h"
#include "../read.h"
#endif

/** @file chain-io/fd/read.h

    This file provides a chain_read overlay for file descriptor reading. Definitions for STDIN_FILENO, STDOUT_FILENO, and STDERR_FILENO are also created if they do not already exist.
 
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

chain_read * chain_read_open_fd (int fd); ///< Creates a chain_read handle for reading the given file descriptor
inline static int chain_read_fd (chain_read * chain) ///< Returns the file descriptor associated with a chain_read created by chain_read_open_fd
{
    return *(int*) chain_read_access_arg(chain);
}
