#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

#define buffer_stream_typedef(buffertype,name)	\
    typedef union { struct { union { struct buffer(buffertype); range_##name range_cast; }; size_t shift; }; buffer_##name buffer_cast; } buffer_stream_##name;

buffer_stream_typedef(void,void);
buffer_stream_typedef(char,char);
buffer_stream_typedef(unsigned char,unsigned_char);
buffer_stream_typedef(char*,string);

#define buffer_stream_shift(buffer_stream, size)	\
    {							\
	buffer_downshift(buffer_stream, size);		\
	(buffer_stream).shift += size;			\
    }

#define buffer_stream_pointer(buffer_stream, position)	\
    ( (buffer_stream).begin - (buffer_stream).shift + (position) )

#define buffer_stream_position(buffer_stream, pointer)	\
    ( (pointer) - (buffer_stream).begin + (buffer_stream).shift )

#define buffer_stream_pointer_check(buffer_stream, pointer)	\
    ( (buffer_stream).begin <= (pointer) && (pointer) < (buffer_stream).end )

