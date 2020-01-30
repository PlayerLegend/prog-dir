#include "hash_table.h"
#include "hash_table_string.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

bool v_tablefunc = false;

int read_lines(table * table, const char * file_name)
{
    FILE * file = fopen(file_name,"r");

    if( !file )
    {
	perror(file_name);
	exit(1);
    }
    
    size_t len = 0;
    char * line = NULL;

    char * end;

    size_t start_size, end_size;
    
    size_t count = 0;

    size_t start_skip;

    while( -1 != getline(&line,&len,file) )
    {
	count++;
	//printf("%d/%zu ",count,size);
	fflush(stdout);
	
	end = strchr(line,'\n');
	if(end)
	    *end = '\0';

	start_skip = table->worst_skip;
	start_size = table->end - table->begin;
	table_include(table,line);
	end_size = table->end - table->begin;
	
	//if( start_size != end_size )
	//    printf("table resized %zu->%zu at %zu, filled %%%3.1f, worst skip %zu->%zu\n",start_size,end_size,count,(float)100 * (float)count / (float)start_size,start_skip,table->worst_skip);

    }

    size_t need = count + 2;
    printf("Count is %zu, table contains %zd values, needs %zu\n",count,table->key.end - table->key.begin, need);
    assert(table->key.end - table->key.begin == (ssize_t)need);

    assert(!ferror(file));
    
    return 0;
}

void test_ptr_table()
{
    table table = { 0 };

    void * values[] = { (void*)12345,
			(void*)838491,
			NULL,
			(void*)12345,
			(void*)838491,
			NULL,
			(void*)127736,
			(void*)127736,
			(void*)3 };

    ssize_t values_len = sizeof(values) / sizeof(*values);

    for(ssize_t i = 0; i < values_len; i++)
    {
	printf("including %p\n",values[i]);
	table_include(&table,values[i]);
    }

    printf("%zd entries in table\n",table.key.end - table.key.begin);
    
    assert(table.key.end - table.key.begin == values_len - 4);
    table_lookup look;
    for(ssize_t i = 0; i < values_len; i++)
    {
	table_find(&look,&table,values[i]);
	assert(look.bucket->state == TABLE_BUCKET_FILLED);
	assert(table_keyof_bucket(&table,look.bucket) == values[i]);
    }
}

int main(int argc, char * argv[])
{
    if( argc != 2 )
	return 1;

    table table = { .config = TABLE_CONFIG_STRING };

    assert(0 != printf("testing ...\n"));

    assert(table_include(&table,"asdf") != table_include(&table,"bcle"));
    assert(table_include(&table,"asdf") != table_include(&table,"bcle"));
    
    char * read_file = argv[1];
    
    read_lines(&table,read_file);

    test_ptr_table();
    
    return 0;
}
