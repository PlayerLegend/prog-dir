#include <string.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../table2/table.h"
#include "../list/list.h"
#include "../keyargs/keyargs.h"
#include "pkg-manifest.h"

keyargs_define(pkg_manifest_add)
{
    table_string_item * table_item = table_string_include(*args.table, args.name);
    list1_element_pkg_manifest_item * list_item = calloc(1, sizeof(*list_item));

    *list_item = (list1_element_pkg_manifest_item)
    {
	
    }
}
