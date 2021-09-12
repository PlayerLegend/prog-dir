#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FLAT_INCLUDES
#include "range.h"
#endif

/**
   @file buffer.h
   Describes a generic buffer data structure and associated operations
   This buffer structure is built on top of the range structure described in range.h, and so the range macros described therein may be used on buffers as well.
   If you are not familiar with range.h you should read over it, as the contents of this file are very closely related to ranges.
*/

#define buffer(type) { struct range(type); type * max; }
/**< @brief The body of a buffer-type structure. It inherits the begin and end members of a range-type structure, and adds a new type pointer member called max, which indicates the end of the memory region allocated to the buffer. Using this max pointer, buffer resizing operations need only realloc the buffer's memory when it grows beyond the max pointer.
 */

#define buffer_typedef(buffertype,name,...)				\
    typedef union { struct buffer(buffertype); range_##name range_cast; __VA_ARGS__; } buffer_##name; \
    typedef union { struct buffer(const buffertype); range_##name range_cast; } buffer_const_##name;
/**<
   @brief Similar to range_typedef, buffer_typedef creates a pair of typedefs, buffer_NAME and buffer_const_NAME, where NAME is provided by the name argument. Both will contain the usual begin, end, and max pointers of a buffer, as well as a range_cast member, which aliases the buffer as a range of its type.
*/

/**
   @union buffer_void
   A buffer of void elements created by buffer_typedef
*/

/**
   @union buffer_char
   A buffer of (signed) chars created by buffer_typedef
*/

/**
   @union buffer_unsigned_char
   A buffer of unsigned chars created by buffer_typedef
*/

/**
   @union buffer_string
   A buffer of char pointers created by buffer_typedef
*/

buffer_typedef(void,void);
buffer_typedef(char,char);
buffer_typedef(unsigned char,unsigned_char, buffer_char char_cast;);
buffer_typedef(char*,string);

int _buffer_resize (buffer_void * expand_buffer, size_t type_size, size_t new_count);

#define buffer_realloc(buffer, count)					\
    (( (size_t)((buffer).max - (buffer).begin) <= (size_t)(count) )	\
     ? _buffer_resize ((buffer_void*)&(buffer), sizeof (*(buffer).begin), (count) * 2) \
     : 0)
/**<
   @brief If the number of members allocated to the buffer is less than count, this macro reallocs the buffer so that it can contain at least 'count' items. Note that this is not the same as resizing the buffer, as this does not change the distance between the begin and end members, only the begin and max members.
*/

#define buffer_resize(buffer, count)			\
    ((0 == buffer_realloc (buffer, count))		\
     ? ((buffer).end = (buffer).begin + (count), 0)	\
     : -1)
/**<
   @brief This macro resizes the given buffer to hold a number of members specified by the count argument, calling buffer_realloc as it needs to in order to allocate sufficient space for the items. Any newly allocated items will be uninitialized.
*/

#define buffer_push(buffer)						\
    ((buffer).end == (buffer).max ? _buffer_resize ( (buffer_void*)&(buffer), sizeof (*(buffer).begin), 10 + ((buffer).max - (buffer).begin) * 3 ), (buffer).end++ : (buffer).end++)
/**<
   @brief This macro pushes a new item onto the end of the given buffer. To do this, it will expand the buffer by one item and evaluate to the address of the newly added item. So to push a value onto a buffer, one may either dereference the call to this macro and initialize the new item directly or store the address given by the macro call in a pointer for later assignment.
*/

#define buffer_rewrite(buffer)			\
    { (buffer).end = (buffer).begin; }
/**<
   @brief Resizes the given buffer to zero, but leaves memory allocated to the buffer in place.
*/

#define buffer_append(buffer, range)					\
    {									\
	size_t add_size = range_count(range);				\
	size_t old_size = range_count(buffer);				\
	size_t new_size = old_size + add_size;				\
	buffer_resize (buffer, new_size);				\
	memcpy ((buffer).begin + old_size, (range).begin, add_size);	\
    }
/**<
   @brief Appends the contents of the given range-like item (this may be another buffer) to the end of the given buffer. The memory referred to by range cannot overlap the memory referred to by buffer
*/

#define buffer_append_n(buffer, ptr, count)			\
    {								\
	size_t add_count = count;		\
	size_t old_count = range_count(buffer);			\
	size_t new_count = old_count + add_count;			\
	buffer_resize (buffer, new_count);			\
	memcpy ((buffer).begin + old_count, ptr, add_count * sizeof(*(ptr))); \
    }
/**<
   @brief Appends a copy of 'count' number of items located at the address 'ptr' to the end of the given buffer. The memory referenced by ptr and count cannot overlap the memory reference by buffer 
*/

#define buffer_copy_n(buffer, ptr, count)				\
    {									\
	size_t new_count = count;						\
	buffer_resize (buffer, new_count);				\
	memcpy ((buffer).begin, ptr, new_count * sizeof(*(ptr)));	\
    }
/**<
   @brief Replaces the contents of the given buffer with a copy of 'count' number of items located at the address 'ptr'. The memory referenced by ptr and count cannot overlap the memory referenced by buffer
*/

void buffer_strcpy (buffer_char * to, const char * input);
/**<
   @brief Replaces the buffer's contents with a copy of the null-terminated string pointed to by 'input'. The number of characters in the resulting buffer will be the number of non-null characters contained in the string, i.e., the count of the buffer will be the same as the count obtained by running strlen on the input string. As such, the terminating byte will be just beyond the buffer's end.
*/

void buffer_strncpy (buffer_char * buffer, const char * begin, size_t count);
/**<
   @brief This function replaces the given char buffer's contents with 'count' bytes from the pointer 'begin' and appends a terminating null byte. The number of items in the buffer will be the number of characters copied from begin, as such this count will not include added null byte. Also note that 'count' bytes MUST be allocated at 'begin', as this function will copy the number of bytes it's told to, regardless of whether a null terminator is found before it's done.
*/

void _buffer_downshift (buffer_void * buffer, size_t element_size, size_t count);

#define buffer_downshift(buffer, count)					\
    _buffer_downshift( (buffer_void*)&(buffer), sizeof(*(buffer).begin), count)
/**<
   @brief Shifts all elements in the buffer beyond the first 'count' elements down to the beginning of the buffer and resizes the buffer so that it contains only these remaining elements. Use this to delete items at the beginning of the buffer.
*/
