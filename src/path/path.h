#ifndef FLAT_INCLUDES
#include <stdbool.h>
#define FLAT_INCLUDES
#endif

#define PATH_SEPARATOR '/'

bool _path_mkdir (char * path, bool make_target);
#define path_mkdir_target(path) _path_mkdir (path, true)
#define path_mkdir_parents(path) _path_mkdir (path, false)
