#include "dictionary.h"
#include "hash_table_string.h"
#include "print.h"
#include "range.h"
#include <string.h>
#include <stdlib.h>

typedef dictionary(unsigned int) count_dict;

void terminate(char * string)
{
    char * end = strchr(string,'\n');
    if(end)
	*end = '\0';
}

enum filter {
    FILTER_NONE,
    FILTER_LE,
    FILTER_GE,
    FILTER_EQ,
};

void print_dictionary(count_dict * dict, enum filter filter_type, unsigned int filter_count)
{
    unsigned int count;
    
    for_range(bucket,dict->keys)
    {
	if(bucket->state != TABLE_BUCKET_FILLED)
	    continue;

	count = dict->values.begin[bucket->key_index];
	
	if(filter_type != FILTER_NONE)
	{
	    if(filter_type == FILTER_LE)
	    {
		if(count > filter_count)
		    continue;
	    }
	    else if(filter_type == FILTER_GE)
	    {
		if(count < filter_count)
		    continue;
	    }
	    else if(filter_type == FILTER_EQ)
	    {
		if(count != filter_count)
		    continue;
	    }
	    else
	    {
		print_error("Invalid filter type");
		exit(1);
	    }
	}
	    

	printf("%u  %s\n",
	       count,
	       dict->keys.key.begin[bucket->key_index]);
    }
}

int main(int argc, char * argv[])
{
    count_dict counts = { 0 };
    dictionary_config(&counts) = TABLE_CONFIG_STRING;

    enum filter filter_type = FILTER_NONE;
    unsigned int filter_count = 0;

    if(argc > 1)
    {
	char * num_string;
	
	if(argc > 2)
	{
	    print_error("Too many arguments, only one is valid (one filter)");
	    exit(1);
	}
	else if(strncmp(argv[1],"le",2) == 0)
	{
	    filter_type = FILTER_LE;
	    num_string = argv[1] + 2;
	}
	else if(strncmp(argv[1],"ge",2) == 0)
	{
	    filter_type = FILTER_GE;
	    num_string = argv[1] + 2;
	}
	else if(strncmp(argv[1],"eq",2) == 0)
	{
	    filter_type = FILTER_EQ;
	    num_string = argv[1] + 2;
	}
	else
	{
	    print_error("Unknown filter type");
	    exit(1);
	}

	filter_count = atoi(num_string);
    }

    

    static struct { size_t len; char * text; } line;

    while( -1 != getline(&line.text,&line.len,stdin) )
    {
	terminate(line.text);
	(*dictionary_access_key(&counts,line.text))++;
    }

    print_dictionary(&counts,filter_type,filter_count);
}
