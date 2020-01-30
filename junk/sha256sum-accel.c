#define FLAT_INCLUDES
#include <stdio.h>
#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "hash_table_string.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

typedef dictionary(size_t) sums_dict;

char * end_line_str(char * line, char * end)
{
    char * find = strstr(line,end);
    if(!find)
	return NULL;

    *find = '\0';
    return find + strlen(end);
}

void end_line_c(char * line, char end)
{
    char * c = strchr(line,end);
    if(c)
	*c = '\0';
}

void load_file(sums_dict * dict, const char * file_name)
{
    FILE * file = fopen(file_name,"r");

    if(!file)
    {
	perror(file_name);
	return;
    }
    
    struct { size_t len; char * text; } line = { 0 };

    char * noise;
    char * sum;
    size_t index;
    
    while( -1 != getline(&line.text,&line.len,file) )
    {
	end_line_c(line.text,'\n');
	noise = line.text;
	sum = end_line_str(noise,"  ");
	index = dictionary_include(dict,sum);
	*dictionary_access_key(dict,noise) = index;
    }

    fclose(file);
}

void test_sum(sums_dict * dict, char * noise)
{
    size_t index = *dictionary_access_key(dict,noise);
    char * sum = dictionary_keyof_index(dict,index);

    printf("%s: %s\n",noise,sum);
}

size_t table_digest_sum(const void * key)
{
    return *(uint64_t*)key;
}

#define TABLE_CONFIG_SUMS			\
    (table_config){				\
	.gen_digest = table_digest_sum,		\
	    .copy = table_copy_string,		\
	    .equals = table_equals_string,	\
	    .free = table_free_string,		\
	    }

int main(int argc, char * argv[])
{
    sums_dict dict = {};

    dictionary_table(&dict).config = TABLE_CONFIG_SUMS;
    
    load_file(&dict,argv[1]);

    test_sum(&dict,"2850c4e7e456b6a0ca61c9685df07a7d89886845ad33a3e7fdea4ee0a31b3094");
    test_sum(&dict,"65c8f173a73739863b3718cea44f311dc349410bf55a4c45217b0ae4c55da028");
}
