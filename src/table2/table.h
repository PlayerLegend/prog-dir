#ifndef FLAT_INCLUDES
#include <string.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#endif

// types

typedef unsigned long long table_digest;

#define table_typedef(query_tag,key_type,table_name,...)		\
    typedef key_type table_##table_name##_key;				\
    typedef const key_type table_##table_name##_const_key;		\
    typedef query_tag table_##table_name##_query table_##table_name##_query; \
    query_tag table_##table_name##_query {					\
	table_digest digest;						\
	table_##table_name##_const_key key;				\
    };									\
    typedef struct table_##table_name##_item table_##table_name##_item; \
    struct table_##table_name##_item {					\
	union {								\
	    struct {							\
		table_digest _digest;					\
		table_##table_name##_key _key;						\
	    };								\
	    table_##table_name##_query query;				\
	};								\
	table_##table_name##_item * next;				\
	struct {							\
	    __VA_ARGS__;						\
	}								\
	    value;							\
    };									\
    typedef table_##table_name##_item** table_##table_name##_bucket;	\
    typedef struct table_##table_name table_##table_name;		\
    struct table_##table_name {						\
	table_digest item_count;					\
	struct range(table_##table_name##_item*);			\
    };

// helpers

table_digest _table_hash_sdbm_string(const char * src);
char * _table_copy_strdup(const char * src);

// table functions

#define table_query_struct(table_name, key) ((table_##table_name##_query) { table_##table_name##_hash(key), key })
#define table_query_union(table_name, key) ((table_##table_name##_query) { .key = key })

#define table_bucket(table_name, table, query)		\
    ((table).begin + ((query).digest % (range_count (table))))	\

#define table_match(table_name, bucket, query)		\
    {									\
	while (*(bucket))						\
	{								\
	    if (table_##table_name##_equals( (*(bucket))->query, query) ) \
	    {								\
		break;							\
	    }								\
	    bucket = &(*(bucket))->next;				\
	}								\
    }

#define table_get_bucket_item(bucket)		\
    (*(bucket))
    
#define table_insert_size_into(table_name, bucket, query, size)		\
    {									\
	*(bucket) = (table_##table_name##_item*) calloc (1, size); \
	(*(bucket))->query = (table_##table_name##_query){ .digest = (query).digest, .key = table_##table_name##_copy((query).key) }; \
    }

#define table_insert_into(table_name, bucket, query) table_insert_size_into (table_name, bucket, query, sizeof(table_##table_name##_item)))

#define for_table_bucket(item,bucket)			\
    for (item = *bucket; item; (item) = (item)->next)

#define for_table(bucket, table)		\
    for_range(bucket,table)

#define table_resize(table_name, table, new_size)			\
    {									\
	table_##table_name old_table = table;				\
	(table).begin = (table_##table_name##_item**) calloc (new_size, sizeof(*(table).begin)); \
	(table).end = (table).begin + (new_size);			\
	table_##table_name##_bucket old_bucket;				\
	table_##table_name##_bucket new_bucket;				\
	table_##table_name##_item * next;				\
	for_table (old_bucket, old_table)				\
	{								\
	    while(*old_bucket)						\
	    {								\
		next = (*old_bucket)->next;				\
		new_bucket = table_##table_name##_bucket(table, (*old_bucket)->query); \
		(*old_bucket)->next = *new_bucket;			\
		*new_bucket = *old_bucket;				\
		*old_bucket = next;					\
	    }								\
	}								\
	free (old_table.begin);						\
    }

#define table_delete(table_name, bucket)			\
    {								\
	table_##table_name##_item * item = *(bucket);		\
	*bucket = item->next;					\
	table_##table_name##_key_free ( item->_key );	\
	table_##table_name##_value_free ( item->value );	\
	free (item);						\
    }

#define table_clear(table_name, table)			\
    {							\
	table_##table_name##_bucket bucket;		\
	for_table(bucket, table)			\
	{						\
	    while (*bucket)				\
	    {						\
		table_##table_name##_delete(bucket);	\
	    }						\
	}						\
	free( (table).begin );				\
	table = (table_##table_name){0};		\
    }						

// generics

#define table_copy_strdup(table_name,a) _table_copy_strdup(a)
#define table_copy_value(table_name,a) ( (table_##table_name##_key)(a) )
#define table_equals_strcmp_0(table_name,query_a,query_b) (0 == strcmp ((const char*)((query_a).key),(const char*)((query_b).key)))
#define table_equals_value(table_name,query_a,query_b) ( (query_a).key == (query_b).key )
#define table_free_pointer(table_name,a) free(a)
#define table_hash_sdbm_string(table_name,a) _table_hash_sdbm_string((const char*)(a))
#define table_hash_value(table_name,a) ((table_digest)(a))
