#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"

#endif


typedef array(size_t) index_array;
typedef dictionary(size_t) function_single;
typedef dictionary(index_array) function_multiple;

typedef struct {
    char * path;
    function_single forward, reverse;
}
    db_single;

typedef struct {
    char * path;
    function_multiple forward;
    function_single reverse;
}
    db_multiple;

#define db_lookup_forward(db_ptr,key)		\
    dictionary_access_key(&(db_ptr)->forward,(void*)table_include(&(db_ptr)->table,(key)))

#define db_lookup_reverse(db_ptr,key)					\
    dictionary_access_key(&(db_ptr)->forward,(void*)table_include(&(db_ptr)->table,(key)))

void init_db();

void db_add_single(db_single * db, size_t key, size_t value);
void db_add_multiple(db_multiple * db, size_t key, size_t value);

int db_load_multiple(db_multiple ** db, const char * file_name);
int db_load_single(db_single ** db, const char * file_name);
