#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FLAT_INCLUDES
#include "range.h"
#endif

#define buffer(type) { struct range(type); type * max; }

#define buffer_typedef(buffertype,name)					\
    typedef union { struct buffer(buffertype); range_##name range_cast; } buffer_##name; \
    typedef union { struct buffer(const buffertype); range_##name range_cast; } buffer_const_##name;

buffer_typedef(void,void);
buffer_typedef(char,char);
buffer_typedef(unsigned char,unsigned_char);
buffer_typedef(char*,string);

int _buffer_resize (buffer_void * expand_buffer, size_t type_size, size_t new_count);

#define buffer_realloc(buffer, count)					\
    {                                                                   \
	if ( (size_t)((buffer).max - (buffer).begin) <= (size_t)(count) ) \
        {                                                               \
            _buffer_resize ((buffer_void*)&(buffer), sizeof (*(buffer).begin), (count) * 2); \
        }                                                               \
    }

#define buffer_resize(buffer, count)			\
    {							\
        buffer_realloc (buffer, count);			\
        (buffer).end = (buffer).begin + (count);        \
    }

#define buffer_push(buffer)						\
    ((buffer).end == (buffer).max ? _buffer_resize ( (buffer_void*)&(buffer), sizeof (*(buffer).begin), 10 + ((buffer).max - (buffer).begin) * 3 ), (buffer).end++ : (buffer).end++)

#define buffer_rewrite(buffer)			\
    { (buffer).end = (buffer).begin; }

#define buffer_append(buffer, range)					\
    {									\
	size_t add_size = range_count(range);				\
	size_t old_size = range_count(buffer);				\
	size_t new_size = old_size + add_size;				\
	buffer_resize (buffer, new_size);				\
	memcpy ((buffer).begin + old_size, (range).begin, add_size);	\
    }

#define buffer_append_n(buffer, ptr, size)			\
    {								\
	size_t add_size = size;					\
	size_t old_size = range_count(buffer);			\
	size_t new_size = old_size + add_size;			\
	buffer_resize (buffer, new_size);			\
	memcpy ((buffer).begin + old_size, ptr, add_size);	\
    }

void _buffer_downshift (buffer_void * buffer, size_t element_size, size_t count);

#define buffer_downshift(buffer, count)					\
    _buffer_downshift( (buffer_void*)&(buffer), sizeof(*(buffer).begin), count)
