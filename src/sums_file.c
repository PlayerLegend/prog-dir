#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "range.h"
#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "hash_table_string.h"
#include "dictionary.h"
#include "sums_file.h"
#include "delimit.h"

static void terminate(char * text)
{
    char * find = strchr(text,'\n');
    if(find)
	*find = '\0';
}

int sums_add(sums_db * db, char * line)
{
    clause_config clause_config =
	{
	    .separator_list = " ",
	    .separator_count = 2,
	};
    
    clause clause;

    terminate(line);
    
    if( -1 == delimit_clause(&clause,&clause_config,line) )
	return -1;

    char * key = dictionary_make_key(db,clause.subject);
    char * value = dictionary_make_key(db,clause.predicate);

    sums_entry * entry;

    entry = dictionary_access_key(db,key);

    for_range(i,entry->value)
    {
	if( 0 == strcmp(*i,value) )
	{
	    return 0;
	}
    }

    *array_push(&entry->value) = value;

    entry = dictionary_access_key(db,value);

    *array_push(&entry->key) = key;

    return 0;
}

int sums_load(sums_db * db, FILE * file)
{
    dictionary_config(db) = TABLE_CONFIG_STRING;

    struct { char * text; size_t len; } line = { 0 };

    while( -1 != getline(&line.text,&line.len,file) )
	sums_add(db,line.text);

    return 0;
}

int sums_dump(FILE * file, sums_db * db)
{
    char * key;
    sums_entry * entry;
    
    for_range(bucket,db->keys)
    {
	if(bucket->state != TABLE_BUCKET_FILLED)
	    continue;

	key = dictionary_keyof_index(db,bucket->key_index);
	entry = dictionary_access_index(db,bucket->key_index);
	
	for_range(value,entry->value)
	{
	    if( 0 > fprintf(file,"%s  %s\n",key,*value) )
		return -1;
	}
    }

    return 0;
}

int sums_delete(sums_db * db, char * line)
{
    clause_config clause_config =
	{
	    .separator_list = " ",
	    .separator_count = 2,
	};
    
    clause clause;

    terminate(line);
    
    if( -1 == delimit_clause(&clause,&clause_config,line) )
	return -1;

    char * key = dictionary_make_key(db,clause.subject);
    char * value = dictionary_make_key(db,clause.predicate);

    sums_entry * entry;

    entry = dictionary_access_key(db,key);
    
    for_range(i,entry->value)
    {
	if( 0 == strcmp(*i,value) )
	{
	    array_flip_del(&entry->value,i);
	    entry = dictionary_access_key(db,value);
	    for_range(i,entry->key)
	    {
		if( 0 == strcmp(*i,value) )
		{
		    array_flip_del(&entry->key,i);
		    break;
		}
	    }
	    return 0;
	}
    }

    return 0;
}
