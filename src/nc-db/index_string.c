#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "stack.h"
#include "array.h"
#include "hash_table.h"
#include "nc-db/index_string.h"
#include "hash_table_string.h"

static table string_table;

static void clean_string_index()
{
    
}

int init_string_index()
{
    string_table.config = TABLE_CONFIG_STRING;
    atexit(clean_string_index);
    if(0 != table_include(&string_table,""))
    {
	return -1;
    }

    return 0;
}

table * get_string_table()
{
    return &string_table;
}
