#include "hash_table_string.h"
#include <string.h>
#include "print.h"
#include <stdlib.h>
#include "range.h"
#include <stdbool.h>

void terminate(char * string)
{
    char * end = strchr(string,'\n');
    if(end)
	*end = '\0';
}

char ** get_flags_end(int argc, char * argv[])
{
    static char ** max;

    if(max)
	return max;

    max = argv + argc;
    
    for( char ** arg = argv + 1; arg < max; arg++ )
    {
	if(strcmp(*arg,"--") == 0)
	{
	    max = arg;
	    break;
	}
    }

    return max; 
}

char * get_flag_arg(int argc, char * argv[], char * flag, char * arg_default)
{
    char ** max = get_flags_end(argc,argv);

    for( char ** arg = argv + 1; arg < max; arg++ )
    {
	if(strcmp(*arg,flag) == 0)
	{
	    if(arg + 1 == max)
	    {
		print_error("The flag %s requires an argument",flag);
		exit(1);
	    }

	    return arg[1];
	}
    }

    return arg_default;
}

enum {
    OP_AND,
    OP_OR,
    OP_NOT,
};

int get_op_type(int argc, char * argv[])
{
    char * operation = get_flag_arg(argc,argv,"--op","and");

    struct op { char * str; int type; };

    const static struct op ops[] = {
	{ "a", OP_AND },
	{ "o", OP_OR },
	{ "i", OP_AND },
	{ "u", OP_OR },
	{ "and", OP_AND },
	{ "or", OP_OR },
	{ "intersection", OP_AND },
	{ "union", OP_OR },
	{ "not", OP_NOT },
	{ "n", OP_NOT },
	{ 0 }
    };

    for( const struct op * op = ops; op->str != NULL; op++ )
    {
	if(0 == strcmp(operation,op->str))
	{
	    return op->type;
	}
    }

    print_error("Unrecognized operation");
    exit(1);
    return -1;
}

void print_table(table * table)
{
    for_range(bucket,*table)
    {
	if(bucket->state != TABLE_BUCKET_FILLED)
	    continue;
	
	printf("%s\n",table->key.begin[bucket->key_index]);
   }
}

bool read_set(table * set, FILE ** streams)
{
    static struct { char * text; size_t len; } line;

    if(!*streams || feof(*streams) || ferror(*streams))
	return false;
    
    while( -1 != getline(&line.text,&line.len,*streams) )
    {
	terminate(line.text);
	
	if( *line.text == '\0' )
	    return true;

	table_include(set,line.text);
    }

    if(feof(*streams))
	streams++;

    return true;
}

void operation_or(FILE ** streams)
{
    table lines_union = { .config = TABLE_CONFIG_STRING };
    
    static struct { char * text; size_t len; } line;

    while(*streams)
    {
	while( -1 != getline(&line.text,&line.len,*streams) )
	{
	    terminate(line.text);

	    if(*line.text == '\0')
		continue;
	    
	    table_include(&lines_union,line.text);
	}

	streams++;
    }

    print_table(&lines_union);
}


void operation_and(FILE ** streams)
{
    table result = { .config = TABLE_CONFIG_STRING };
    table set = { .config = TABLE_CONFIG_STRING };
    table_lookup find;
    char * key;
    
    if(!read_set(&result,streams))
	return;

    while(read_set(&set,streams))
    {
	for_range(bucket,result)
	{
	    if(bucket->state != TABLE_BUCKET_FILLED)
		continue;
	    
	    key = result.key.begin[bucket->key_index];
	    
	    table_find(&find,&set,key);
	    if(find.bucket->state != TABLE_BUCKET_FILLED)
	    {
		table_find(&find,&result,key);
		table_delete(find);
	    }
	}
	table_clear(&set);
    }

    print_table(&result);
}

void operation_not(FILE ** streams)
{
    table result = { .config = TABLE_CONFIG_STRING };
    table set = { .config = TABLE_CONFIG_STRING };
    table_lookup find;
    char * key;
    
    if(!read_set(&result,streams))
	return;

    while(read_set(&set,streams))
    {
	for_range(bucket,result)
	{
	    if(bucket->state != TABLE_BUCKET_FILLED)
		continue;
	    
	    key = result.key.begin[bucket->key_index];
	    
	    table_find(&find,&set,key);
	    if(find.bucket->state == TABLE_BUCKET_FILLED)
	    {
		table_find(&find,&result,key);
		table_delete(find);
	    }
	}
	table_clear(&set);
    }

    print_table(&result);
}

int main(int argc, char * argv[])
{
    FILE * streams[2] = { stdin, NULL };

    switch(get_op_type(argc,argv))
    {
    case OP_OR:
	operation_or(streams);
	return 0;
    case OP_AND:
	operation_and(streams);
	return 0;
    case OP_NOT: // subsequent sets remove elements from the first
	operation_not(streams);
	return 0;
    default:
	print_error("Invalid operation type");
	return 1;
    }

    print_error("main should not reach here");
    return 1;
}
