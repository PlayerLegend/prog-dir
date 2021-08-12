#ifndef FLAT_INCLUDES
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../list/list.h"
#endif

//#define TABLE_STRING
#ifdef TABLE_STRING
#define table_key char*
#include <string.h>
inline static table_key _table_key_copy(const table_key src)
{
    return strcpy (malloc (strlen (src) + 1), src);
}
#define table_key_copy(dst,src) dst = _table_key_copy (src)
inline static size_t _table_key_hash(const table_key src)
{
    size_t digest = 0;
    int c;
    
    while ( (c = *src++) )
    {
	digest = c + (digest << 6) + (digest << 16) - digest;
    }

    return digest;
}
#define table_key_hash(digest,src) digest = _table_key_hash (src)
#define table_key_equals(a,b) (0 == strcmp (a, b))
#define table_key_free(key) free (key)
#endif

#ifndef table_key
#define table_key int
#define table_key_copy(dst,src) (dst) = (src)
#define table_key_hash(digest,src) (digest) = (src)
#define table_key_equals(a,b) (a) == (b)
#define table_key_free(key)
#endif

#ifndef TABLE_VALUE
#define TABLE_VALUE
#endif

#ifdef TABLE_IMMUTABLE_STRING
#define table_key const char*
#include <string.h>
inline static table_key _table_key_copy(const table_key src)
{
    return src;
}
#define table_key_copy(dst,src) dst = _table_key_copy (src)
inline static size_t _table_key_hash(const table_key src)
{
    return (size_t) src;
}
#define table_key_hash(digest,src) digest = (size_t) src
#define table_key_equals(a,b) (a == b)
#define table_key_free(key) 
#endif

typedef struct list_element (struct { size_t digest; table_key key; TABLE_VALUE; }) table_element;
typedef list_handle (table_element) table_bucket;
typedef struct { size_t used; struct range(table_bucket); } table;
typedef struct table_result table_result;
struct table_result {
    const table_key key;
    table * table;
    size_t digest;
    table_element * match;
    table_bucket * bucket;
};

#define for_table(item, bucket, table)		\
    for_range (bucket, table)			\
	for_list (item, *bucket)

static void table_resize (table * resize_table, size_t new_bucket_count)
{
    table old_table = *resize_table;
    range_calloc (*resize_table, new_bucket_count);

    table_element * element;
    table_bucket * new_bucket;
    table_bucket * old_bucket;
    
    for_range (old_bucket, old_table)
    {
	while ( (element = list_pop (*old_bucket)) )
	{
	    new_bucket = resize_table->begin + element->child.digest % new_bucket_count;
	    list_push (*new_bucket, *element);
	}
    }

    free (old_table.begin);
}

inline static void table_autosize (table * table)
{
    if (80 * (size_t) range_count (*table) <= 100 * (table)->used)
    {
	table_resize (table, (1 + 1.618034) * range_count (*table) + 107);
    }
}

static void table_search (table_result * result, table * table, const table_key key)
{
    table_autosize (table);

    *(table_key*) &result->key = (table_key) key;
    result->table = table;
    table_key_hash (result->digest, key);
    result->bucket = table->begin + result->digest % range_count (*table);

    table_element * element;
    
    for_list (element, *result->bucket)
    {
	if (table_key_equals (key, element->child.key))
	{
	    result->match = element;
	    return;
	}
    }
    
    result->match = NULL;
}

static void table_add (table_result * result)
{
    assert (!result->match);
    result->match = calloc (1, sizeof (*result->match));
    table_key_hash(result->match->child.digest, result->key);
    table_key_copy(result->match->child.key, result->key);
    list_push (*result->bucket, *result->match);
}

static void table_remove (table_result * result)
{
    assert (result->match);
    table_element ** parent = result->bucket;
    while (*parent != result->match)
    {
	parent = (table_element**) &(**parent).peer;	
    }

    assert (*parent == result->match);

    *parent = (**parent).peer;

    assert (*parent != result->match);
}

inline static const table_key table_include (table * table, const table_key key)
{
    table_result result;
    table_search (&result, table, key);
    if (!result.match)
    {
	table_add (&result);
    }
    return result.match->child.key;
}

inline static table_element * table_include_element (table * table, const table_key key)
{
    table_result result;
    table_search (&result, table, key);
    if (!result.match)
    {
	table_add (&result);
    }
    return result.match;
}

inline static void table_exclude (table * table, const table_key key)
{
    table_result result;
    table_search (&result, table, key);
    if (result.match)
    {
	table_remove(&result);
    }
}

inline static void table_clear (table * clear_table)
{
    table_element * element;
    table_bucket * bucket;
    
    for_range (bucket, *clear_table)
    {
	while ( (element = list_pop(*bucket)) )
	{
	    table_key_free (element->child.key);
	    free (element);
	}
    }

    free (clear_table->begin);

    *clear_table = (table){0};
}
