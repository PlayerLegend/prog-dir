#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "array.h"

#endif

typedef struct
{
    size_t key_index;
    size_t digest;
    enum { TABLE_BUCKET_EMPTY, TABLE_BUCKET_DELETED, TABLE_BUCKET_FILLED } state;
}
    table_bucket;

typedef struct
{
    size_t (*gen_digest)(const void * key);
    int (*copy)(void ** dst_key, const void * src_key);
    void (*free)(void * key);
    int (*equals)(const void * a, const void * b);
}
    table_config;

typedef struct
{
    array(void*) key;
    array(size_t) deleted_index;
    table_bucket * begin;
    table_bucket * end;
    size_t worst_skip;
    table_config config;
}
    table;

typedef struct
{
    table_bucket * bucket;
    const void * key;
    table * table;
}
    table_lookup;

void table_find(table_lookup * look, table * in, const void * key);
int table_fill(const table_lookup lookup);
int table_delete(table_lookup lookup);
size_t table_include(table * in, const void * key);
void table_exclude(table * from, void * key);
void table_copy(table * dst, table * src);
void table_clear(table * clear);

#define table_keyof_index(tablep,index)		\
    ((tablep)->key.begin[index])

#define table_keyof_bucket(tablep,bucketp)	\
    table_keyof_index(tablep,(bucketp)->key_index)

#define table_keycount(tablep)	\
    ((tablep)->key.end - (tablep)->key.begin)


inline static void * table_make_key(table * table, void * from)
{
    size_t index = table_include(table,from);
    return table_keyof_index(table,index);
}
