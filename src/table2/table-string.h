#ifndef FLAT_INCLUDES
#include <string.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "table.h"
#endif

#ifndef table_string_value
#define table_string_value
#endif

table_typedef(struct, char*,string, table_string_value);
void _table_string_resize(table_string * table, table_digest new_size);
table_string_item * _table_string_include (table_string * table, const char * key, size_t size);
#define table_string_hash(a) table_hash_sdbm_string(string,a)
#define table_string_equals(query_a,query_b) table_equals_strcmp_0(string,query_a,query_b)
#define table_string_key_free(a) table_free_pointer(string,a)
#define table_string_copy(a) table_copy_strdup(string,a)
#define table_string_query(key) table_query_struct(string, key)
#define table_string_bucket(table,query) table_bucket(string,table,query)
#define table_string_match(bucket,query) table_match(string,bucket,query)
#define table_string_insert_into(bucket,query) table_insert_into(string,bucket,query)
#define table_string_resize(table,new_size) _table_string_resize (&(table), new_size)
#define table_string_include(table, key) _table_string_include (&(table), key, sizeof(table_string_item))
#define table_string_delete(bucket) table_delete(string, bucket)
#define table_string_clear(table) table_clear(string, table)

#ifndef table_string_value_free
#define table_string_value_free(a)
#endif
