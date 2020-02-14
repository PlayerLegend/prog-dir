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
    size_t key, value;
}
    key_value;

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

int init_db();

size_t db_index_string(const char * string);
int db_make_kv(key_value * kv, char * line);
void db_add_single(db_single * db, key_value kv);
void db_delete_single(db_single * db, key_value kv);
int db_load_single(db_single ** db, const char * file_name);

void db_add_multiple(db_multiple * db, key_value kv);
void db_delete_multiple(db_multiple * db, key_value kv);
int db_load_multiple(db_multiple ** db, const char * file_name);

#define db_lookup_forward(db_ptr,key)		\
    (*dictionary_access_key(&(db_ptr)->forward,(void*)(key)))

#define db_lookup_reverse(db_ptr,key)					\
    (*dictionary_access_key(&(db_ptr)->reverse,(void*)(key)))
