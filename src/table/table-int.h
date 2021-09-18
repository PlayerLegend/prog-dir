#ifndef FLAT_INCLUDES
#include <string.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "table.h"
#endif

#ifndef table_int_value
#define table_int_value
#endif

table_typedef(int,int, table_int_value);
#define table_int_copy(a) table_copy_value(int,a)
#define table_int_hash(a) table_hash_value(int,a)
#define table_int_equals(a,b) table_equals_value(int,a,b)
#define table_int_free(key)
