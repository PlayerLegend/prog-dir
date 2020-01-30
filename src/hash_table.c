#include "hash_table.h"
#include <stdlib.h>
#include <stdio.h>
#include "print.h"
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "range.h"

#define GOLDEN_RATIO 1.61803398875
#define INITIAL_PRIME 27
#define MAX_KEY_PERCENT 80
#define MAX_WORST_SKIP_PERCENT 10
#define GROW_RATE (GOLDEN_RATIO + 2)

size_t next_size(size_t size)
{
    if (0 == size)
    {
	return INITIAL_PRIME;
    }
    else
    {
	size *= GROW_RATE;

	for(size_t i = 2; i < INITIAL_PRIME; i++)
	{
	    if(size % i == 0)
	    {
		i = 2;
		size++;
	    }
	}

	return size;
    }
}

table_bucket * _table_find(table * in, const void * key, size_t digest);

int table_grow(table * grow)
{
    struct
    {
	table_bucket * begin;
	table_bucket * end;
    }
    orig = { grow->begin, grow->end };

    grow->worst_skip = 0;
    
    size_t new_size = next_size(grow->end - grow->begin);

    grow->begin = calloc(new_size,sizeof(*grow->begin));

    if( grow->begin == NULL )
    {
	perror(__func__);
	return 0;
    }

    grow->end = grow->begin + new_size;

    table_bucket * src;

    table_bucket * dst;
    
    for(src = orig.begin; src != orig.end; src++)
    {
	if(src->state != TABLE_BUCKET_FILLED)
	    continue;
	
	dst = _table_find(grow,grow->key.begin + src->key_index,src->digest);

	*dst = *src;
    }

    return 0;
}

inline static bool _equals(table * in, const void * a, const void * b)
{
    if (in->config.equals)
    {
	return in->config.equals(a,b);
    }
    else
    {
	return a == b;
    }
}

table_bucket * _table_find(table * in, const void * key, size_t digest)
{
    size_t size = in->end - in->begin;

    if( 100 * (size_t)(in->key.end - in->key.begin) >= MAX_KEY_PERCENT * size ||
	100 * in->worst_skip >= MAX_WORST_SKIP_PERCENT * size )
    {
	table_grow(in);
    }
    
    size = in->end - in->begin;

    size_t index = digest % size;

    table_bucket * point;

    table_bucket * first_deleted = NULL;

    size_t count = 0;

    for (size_t ofs = 0; ofs < size; ofs++)
    {
	count++;
	
	point = in->begin + (index + ofs) % size;

	if (point->state == TABLE_BUCKET_FILLED)
	{
	    if (point->digest == digest && _equals(in,key,in->key.begin[point->key_index]))
	    {
		if (in->worst_skip < count)
		    in->worst_skip = count;
		
		return point;
	    }
	    else
	    {
		continue;
	    }
	}
	else if (point->state == TABLE_BUCKET_DELETED)
	{
	    if (NULL == first_deleted)
		first_deleted = point;
	}
	else // empty
	{
	    if (NULL != first_deleted)
		point = first_deleted;

	    point->digest = digest;

	    if (in->worst_skip < count)
		in->worst_skip = count;
	        
	    return point;
	}
    }

    print_error("ran out of indexes, shouldn't be possible, it's broke");
    
    return NULL;
}

void table_find(table_lookup * look, table * in, const void * key)
{
    size_t digest;
    if (in->config.gen_digest)
	digest = in->config.gen_digest(key);
    else
	digest = (size_t)key * 157;
    
    *look = (table_lookup){ _table_find(in,key,digest), key, in };
}

int table_fill(table_lookup lookup)
{
    assert(lookup.bucket->state != TABLE_BUCKET_FILLED);
    
    void * set;

    if (lookup.table->config.copy)
    {
	if (-1 == lookup.table->config.copy(&set,lookup.key))
	{
	    print_error("copy failed");
	    return -1;
	}
    }
    else
    {
	set = (void*)lookup.key;
    }

    void ** new_key;
    
    if (stack_not_empty(&lookup.table->deleted_index))
	new_key = lookup.table->key.begin + *array_pop(&lookup.table->deleted_index);
    else
	new_key = array_push(&lookup.table->key);
    
    *new_key = set;
    lookup.bucket->key_index = new_key - lookup.table->key.begin;
    lookup.bucket->state = TABLE_BUCKET_FILLED;

    return 0;
}

int table_delete(table_lookup lookup)
{
    if (lookup.table->config.free)
	lookup.table->config.free(lookup.table->key.begin[lookup.bucket->key_index]);

    lookup.bucket->state = TABLE_BUCKET_DELETED;

    return 0;
}

size_t table_include(table * in, const void * key)
{
    table_lookup look;
    table_find(&look,in,key);

    if (look.bucket->state != TABLE_BUCKET_FILLED)
	table_fill(look);

    return look.bucket->key_index;
}

void table_exclude(table * from, void * key)
{
    table_lookup look;

    table_find(&look,from,key);

    if(look.bucket->state == TABLE_BUCKET_FILLED)
	table_delete(look);
}

void table_copy(table * dst, table * src)
{
    *dst = *src;
    size_t bucket_count = src->end - src->begin;
    dst->begin = malloc(sizeof(*src->begin) * bucket_count);
    dst->end = dst->begin + bucket_count;
    array_copy(&dst->key,&src->key);
    array_copy(&dst->deleted_index,&src->deleted_index);
}

void table_clear(table * clear)
{
    memset(clear->begin,0,(clear->end - clear->begin) * sizeof(*clear->begin));
    if(clear->config.free)
    {
	for_range(key,clear->key)
	    clear->config.free(*key);
    }
    array_rewrite(&clear->key);
    array_rewrite(&clear->deleted_index);
    clear->worst_skip = 0;
}
