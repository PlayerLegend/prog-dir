#include "sums_file.h"
#include <string.h>
#include <stdlib.h>
#include "hash_table_string.h"
#include "range.h"
#include "indexof.h"
#include <assert.h>

void sums_db_init(sums_db * init)
{
    *init = (sums_db){ .keys.config = TABLE_CONFIG_STRING };
}

void sums_db_add(sums_db * addto, const char * sum, const char * name)
{
    struct { size_t sum, name; }
    index = { dictionary_indexof_key(addto,sum),
	      dictionary_indexof_key(addto,name) };

    table_include
	( &dictionary_access_index(addto,index.sum)->names,
	  (void*)index.name );
    
    table_include
	( &dictionary_access_index(addto,index.name)->sums,
	  (void*)index.sum );

/*    printf("sums size: %zd, names size: %zd\n",
	   table_keycount(&dictionary_access_index(addto,index.sum)->names),
	   table_keycount(&dictionary_access_index(addto,index.name)->sums));

    printf("Sum: %zd, name: %zd\n",index.sum,index.name);
    printf("count: %zd\n", table_keycount(&addto->key_table)); */
    
    assert( index.sum == dictionary_indexof_key(addto,sum) );
    assert( index.name == dictionary_indexof_key(addto,name) );
}

void sums_db_del_line(sums_db * delfrom, const char * sum, const char * name)
{
    struct { size_t sum, name; }
    index = { dictionary_indexof_key(delfrom,sum),
	      dictionary_indexof_key(delfrom,name) };

    table_exclude
	( &dictionary_access_index(delfrom,index.sum)->names,
	  (void*)index.name );
    
    table_exclude
	( &dictionary_access_index(delfrom,index.name)->sums,
	  (void*)index.sum );
    
    assert( index.sum == dictionary_indexof_key(delfrom,sum) );
    assert( index.name == dictionary_indexof_key(delfrom,name) );
}

void sums_db_del_sum(sums_db * delfrom, const char * sum)
{
    sums_relation * sum_relation = dictionary_access_key(delfrom,sum);
    size_t sum_index = dictionary_indexof_value(delfrom,sum_relation);

    table * sums_table = &sum_relation->names;
    
    for_range(i,*sums_table)
    {
	if(i->state != TABLE_BUCKET_FILLED)
	    continue;

	table_exclude(&dictionary_access_index(delfrom,
					       *(size_t*)table_keyof_bucket(sums_table,i))->sums,
		      (void*)sum_index);
    }
}

void sums_db_del_name(sums_db * delfrom, const char * name)
{
    sums_relation * name_relation = dictionary_access_key(delfrom,name);
    size_t name_index = dictionary_indexof_value(delfrom,name_relation);

    for_range(i,name_relation->sums)
    {
	if(i->state != TABLE_BUCKET_FILLED)
	    continue;

	table_exclude(&dictionary_access_index(delfrom,i->key_index)->names,(void*)name_index);
    }
}

int sums_file_load(sums_db * load, FILE * file)
{
    char * line = NULL;
    size_t len = 0;

    char * name;
    char * sum;
    char * end;

    while(-1 != getline(&line,&len,file))
    {
	end = strchr(line,'\n');
	if(end)
	    *end = '\0';

	sum = line;
	end = strchr(sum,' ');
	if(end)
	    *end = '\0';
	else
	    continue;

	end++;
	if(*end != ' ')
	    continue;
	end++;
	if(*end == '\0')
	    continue;

	name = end;

	//printf("Adding %s ==== %s\n",sum,name);
	
	sums_db_add(load,sum,name);
    }
    
    free(line);
    
    return 0;
}
