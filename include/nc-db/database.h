#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "hash_table_string.h"
#include "dictionary.h"

#endif

typedef size_t db_handle;

int db_load(db_handle * handle, const char * path);
int db_add(db_handle handle, char * line);
int db_delete(db_handle handle, char * line);
void db_get_values(char_array * output, db_handle handle, const char * key);
void db_get_keys(char_array * output, db_handle handle, const char * value);
int db_dump(db_handle handle);
