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

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

io_write * io_write_open_fd (int fd);
