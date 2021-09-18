#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "table.h"
#include "table-string.h"

table_digest _table_hash_sdbm_string(const char * src)
{
    table_digest digest = 0;
    int c;
    
    while ( (c = *src++) )
    {
	digest = c + (digest << 6) + (digest << 16) - digest;
    }

    return digest;
}

char * _table_copy_strdup(const char * src)
{
    return strcpy (malloc (strlen (src) + 1), src);
}

void _table_string_resize(table_string * table, table_digest new_size)
{
    table_resize (string, *table, new_size);
}

table_string_item * _table_string_include (table_string * table, const char * key, size_t size)
{
    if (100 * table->item_count >= 80 * (table_digest) range_count (*table))
    {
	table_string_resize (*table, table->item_count ? (table->item_count * 2.71828) : 1031);
    }

    assert (!range_is_empty (*table));
    
    table_string_query query = table_string_query(key);
    table_string_bucket bucket = table_string_bucket(*table, query);

    table_string_match(bucket, query);

    if (*bucket)
    {
	return *bucket;
    }
    else
    {
	table_insert_size_into(string, bucket, query, size);

	table->item_count++;
	
	assert (*bucket);
	assert ( (*bucket)->query.key != key );
	assert ( 0 == strcmp ( (*bucket)->query.key, key ) );

	return *bucket;
    }
}
