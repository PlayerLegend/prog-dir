#include "index_map.h"
#include <stdlib.h>
#include <string.h>

void * _index_map_access(index_map_char * table, size_t element_size, size_t index)
{
    char * next = table->begin + (index + 1) * element_size;

    if( next >= table->alloc )
    {
	size_t have = table->alloc - table->begin;
	size_t need = (next - table->begin) * 2;
	if(need < 32)
	    need = 32;
	table->begin = realloc(table->begin,need);
	table->alloc = table->begin + need;
	memset(table->begin + have, 0, need - have);
    }

    return table->begin + element_size * index;
}
