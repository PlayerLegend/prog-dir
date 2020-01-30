#include "dictionary.h"
#include "hash_table_string.h"
#include "assert.h"

void string_dictionary()
{
    dictionary(int) dict = { .keys.config = TABLE_CONFIG_STRING };

    *dictionary_access_key(&dict,"asdf") = 100;
    *dictionary_access_key(&dict,"bcle") = 150;

    printf("asdf: %d\n",*dictionary_access_key(&dict,"asdf"));
    printf("bcle: %d\n",*dictionary_access_key(&dict,"bcle"));    
}

void ptr_dictionary()
{
    dictionary(int) dict = { 0 };

    struct value_pair { void * key; int value; } pairs[] = {
	{ (void*)12345, 12 },
	{ (void*)11123, 14 },
	{ (void*)808080, 57 },
	{ (void*)8080809, 8 },
    };
    
    ssize_t pairs_len = sizeof(pairs) / sizeof(*pairs);

    for( int i = 0; i < pairs_len; i++ )
    {
	*dictionary_access_key(&dict,pairs[i].key) = pairs[i].value;
    }
    
    for( int i = 0; i < pairs_len; i++ )
    {
	int value = *dictionary_access_key(&dict,pairs[i].key);
	printf("for %p, %d == %d\n",pairs[i].key,value,pairs[i].value);
	fflush(stdout);
	assert(value == pairs[i].value);
	printf("yes\n");
	fflush(stdout);
    }
}

int main()
{
    string_dictionary();
    ptr_dictionary();
}
