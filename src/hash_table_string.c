#include "hash_table_string.h"
#include <string.h>
#include <stdlib.h>

size_t table_digest_string(const void * key)
{
    size_t ret = 0;
    char c;

    const char * key_c = key;

    while( (c = *key_c++) )
	ret = c + (ret << 6) + (ret << 16) - ret;

    return ret;
}

int table_equals_string(const void * a, const void * b)
{
    char ac, bc;
    const char * as = a;
    const char * bs = b;

    while(1)
    {
	ac = *as++;
	bc = *bs++;
	if(!ac && !bc)
	    return 1;

	if(ac != bc)
	    return 0;
    }

    return 0;
}

int table_copy_string(void ** dst, const void * src)
{
    char * new = malloc(strlen(src) + 1);
    if(!new)
    {
	perror(__func__);
	return -1;
    }
    
    *dst = strcpy(new,src);
    
    return 0;
}

void table_free_string(void * key)
{
    free(key);
}
