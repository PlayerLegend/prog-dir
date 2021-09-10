#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "pkg.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#endif

#include "../table2/table.h"
#define table_string_value const char* package_name;
#include "../table2/table-string.h"

#define LOG_PATH "pkg/system.log"

struct pkg_root {
    struct {
	buffer_char name;
	buffer_char path;
	buffer_char build_sh;
    }
	tmp;
    
    buffer_char path;
    table_string log;
    buffer_const_string protected_paths;
};

inline static bool file_exists(const char * path)
{
    struct stat s;
    return 0 == stat (path, &s);
}
