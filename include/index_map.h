#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#endif

#define index_map(type)				\
    struct { type * begin; type * alloc; }

typedef index_map(char) index_map_char;

void * _index_map_access(index_map_char * table, size_t element_size, size_t index);

#define index_map_access(map_p,index)					\
    ((typeof((map_p)->begin)) _index_map_access((void*)(map_p),sizeof(*(map_p)->begin),index))
