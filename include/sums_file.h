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

typedef struct {
    string_array key;
    string_array value;
}
    sums_entry;

typedef dictionary(sums_entry) sums_db;

int sums_load(sums_db * db, FILE * file);
int sums_add(sums_db * db, char * line);
int sums_delete(sums_db * db, char * line);
int sums_dump(FILE * file, sums_db * db);
