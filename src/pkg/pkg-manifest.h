#ifndef FLAT_INCLUDES
#include <string.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../table2/table.h"
#include "../list/list.h"
#include "../keyargs/keyargs.h"
#endif

typedef struct {
    const char * version;
    const char * arch;
    const char * metahash;
}
    pkg_manifest_item;

list1_typedef(pkg_manifest_item, pkg_manifest_item);

#define table_string_value list1_handle_pkg_manifest_item item;
#include "../table2/table-string.h"

keyargs_declare(void, pkg_manifest_add,
		table_string * table;
		const char * name;
		const char * version;
		const char * arch;
		const char * metahash;);

#define pkg_manifest_add(...) keyargs_call(pkg_manifest_add,__VA_ARGS__)

