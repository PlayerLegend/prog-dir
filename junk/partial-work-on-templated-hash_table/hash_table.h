#ifndef FLAT_INCLUDES
#include "array.h"
#endif

typedef struct
{
    size_t key_index;
    size_t digest;
    enum { TABLE_BUCKET_EMPTY, TABLE_BUCKET_DELETED, TABLE_BUCKET_FILLED } state;
}
    table_bucket;

#define table_config(type)					\
    struct {							\
	size_t (*gen_digest)(const type * key);			\
	int (*copy)(type * dst_key, const type * src_key);	\
	void (*free)(type * key);				\
	int (*equals)(const type * a, const type * b);		\
    }

#define table(type)				\
    struct {					\
	array(type) key;			\
	table_config(type) config;			\
	array(size_t) deleted_index;		\
	table_bucket * begin;			\
	table_bucket * end;			\
	size_t worst_skip;			\
    }

#define table_result(type)			\
    struct {					\
	table_bucket * bucket;			\
	const type * key;			\
    }

typedef table(void) void_table;


table_bucket * _table_find_bucket(void_table * in, const void * key, size_t key_size);
int _table_fill_bucket(void_table * in, table_bucket * bucket, const void * key, size_t key_size);
int _table_clear_bucket(void_table * in, table_bucket * bucket, const void * key, size_t key_size);

#define table_lookup(result,table,key) {	\
	(result).key = &(key);						\
	(result).bucket = _table_find_bucket((void*)&(table),&(key),sizeof(key)); \
    }

#define table_insert(table,result)				\
    _table_fill_bucket((void*)&(table),(result).bucket,(void*)(result).key)

#define table_include(table,key)		\
    _table_fill_bucket((void*)&(table),_table_find_bucket((void*)&(table),(void*)&(key),sizeof(key)),(void*)&(key))

/*
void table_find(table_lookup * look, table * in, const void * key);
int table_fill(const table_lookup lookup);
int table_delete(table_lookup lookup);
size_t table_include(table * in, const void * key);
void table_exclude(table * from, void * key);
*/
#define table_keyof_index(tablep,index)		\
    ((tablep)->key.begin[index])

#define table_keyof_bucket(tablep,bucketp)	\
    table_keyof_index(tablep,(bucketp)->key_index)
