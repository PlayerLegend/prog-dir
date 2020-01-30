#include "hash_table.h"
#include <stdlib.h>
#include <stdio.h>
#include "print.h"
#include <stdbool.h>
#include <assert.h>

#define GOLDEN_RATIO 1.61803398875
#define INITIAL_PRIME 27
#define MAX_KEY_PERCENT 80
#define MAX_WORST_SKIP_PERCENT 10
#define GROW_RATE (GOLDEN_RATIO + 2)

static size_t _next_size(size_t size)
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

static table_bucket * _choose_bucket(void_table * in, const void * key, size_t key_size, size_t key_digest);

static int _grow(void_table * grow, size_t key_size)
{
    struct
    {
	table_bucket * begin;
	table_bucket * end;
    }
    orig = { grow->begin, grow->end };

    grow->worst_skip = 0;
    
    size_t new_size = _next_size(grow->end - grow->begin);

    grow->begin = calloc(new_size,sizeof(*grow->begin));

    if( grow->begin == NULL )
    {
	perror(__func__);
	return -1;
    }

    grow->end = grow->begin + new_size;

    table_bucket * src;

    table_bucket * dst;
    
    for(src = orig.begin; src != orig.end; src++)
    {
	if(src->state != TABLE_BUCKET_FILLED)
	    continue;
	
	dst = _choose_bucket(grow,grow->key.begin + src->key_index,key_size,src->digest);

	*dst = *src;
    }

    return 0;
}

static table_bucket * _choose_bucket(void_table * in, const void * key, size_t key_size, size_t key_digest)
{
    size_t size = in->end - in->begin;
    
    if( 100 * (size_t)(in->key.end - in->key.begin) >= MAX_KEY_PERCENT * size ||
	100 * in->worst_skip >= MAX_WORST_SKIP_PERCENT * size )
    {
	_grow(in,key_size);
    }
    
    size = in->end - in->begin;

    size_t start = key_digest % size;

    table_bucket * point;
    table_bucket * first_deleted = NULL;

    size_t count = 0;

    for(size_t skip = 0; skip < size; skip++)
    {
	count++;
	point = in->begin + (start + skip) % size;
	
	if (point->state == TABLE_BUCKET_FILLED)
	{
	    if (point->digest == key_digest && in->config.equals(key,(const char*)in->key.begin + key_size * point->key_index))
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

	    point->digest = key_digest;

	    if (in->worst_skip < count)
		in->worst_skip = count;
	        
	    return point;
	}
    }

    print_error("ran out of indexes, shouldn't be possible, it's broke");
    
    return NULL;
}

table_bucket * _table_find_bucket(void_table * in, const void * key, size_t key_size)
{
    return _choose_bucket(in, key, key_size, in->config.gen_digest(key));
}

int _table_fill_bucket(void_table * in, table_bucket * bucket, const void * key, size_t key_size)
{
    if (bucket->state == TABLE_BUCKET_FILLED)
	return bucket->key_index;
    
    void * key_copy;
    
    if (stack_not_empty(&in->deleted_index))
	key_copy = (char*)in->key.begin + *array_pop(&in->deleted_index) * key_size;
    else
	key_copy = array_push(&in->key);
    
    if (-1 == in->config.copy(key_copy,key))
    {
	print_error("copy failed");
	return -1;
    }
    
    bucket->state = TABLE_BUCKET_FILLED;

    bucket->key_index = key_copy - in->key.begin;

    return bucket->key_index;
}

int _table_clear_bucket(void_table * in, table_bucket * bucket, const void * key, size_t key_size)
{
    if (in->config.free)
	in->config.free((char*)in->key.begin + key_size * bucket->key_index);

    bucket->state = TABLE_BUCKET_DELETED;
}
