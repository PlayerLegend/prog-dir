#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stddef.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

#define PATH_SEPARATOR '/'

bool _path_mkdir (char * path, bool make_target);
#define path_mkdir_target(path) _path_mkdir (path, true)
#define path_mkdir_parents(path) _path_mkdir (path, false)
void path_cat (buffer_char * buffer, const char * a, const char * b);
