#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>

#include "range.h"
#include "stack.h"

#endif

#define array(type)				\
    union {					\
	stack _stack;				\
	struct {				\
	    type * begin;			\
	    type * end;				\
	};					\
    }

#define compact_array(type)			\
    struct {					\
	array(type);				\
	array(size_t) deleted_index;		\
    }

#define array_count(arrayp)			\
    ((arrayp)->end - (arrayp)->begin)

#define array_rewrite(arrayp)			\
    stack_rewrite(&(arrayp)->_stack)

#define array_forget(arrayp)			\
    stack_forget(&(arrayp)->_stack)

#define array_indexof(arrayp,valuep)		\
    ((valuep) - (arrayp)->begin)

#define array_push(arrayp)		\
    (stack_push_type(&(arrayp)->_stack,typeof(*(arrayp)->begin)))

#define compact_array_push(arrayp)		\
    ( is_range_empty((arrayp)->deleted_index) ? array_push(arrayp) : (arrayp)->begin + *array_pop(&(arrayp)->deleted_index) )

#define compact_array_delete(arrayp,index)	\
    ( array_push(&(arrayp)->deleted_index,index) )

#define array_pop(arrayp)		\
    (stack_pop_type(&(arrayp)->_stack,typeof(*(arrayp)->begin)))

#define array_flip_del(arrayp,ptr)				\
    *(ptr) = *array_pop(arrayp);

#define  array_rewrite(arrayp)			\
    stack_rewrite(&(arrayp)->_stack)

#define array_import_alloc(arrayp,abegin,aend)	\
    stack_import_alloc(&(arrayp)->_stack,abegin,aend)

#define array_copy(dstp,srcp)			\
    stack_copy(&(dstp)->_stack,&(srcp)->_stack)

#define array_append_several(arrayp,start,count)			\
    {									\
	stack_push(&(arrayp)->_stack,sizeof(*(arrayp)->begin) * (count)); \
	memcpy((arrayp)->end - (count),start,(count) * sizeof(*(arrayp)->begin)); \
    }
    
#define array_alloc(arrayp,n)			\
    stack_alloc(&(arrayp)->_stack,(n) * (sizeof(*(arrayp)->begin)))

#define array_write_several(arrayp,start,count)	\
    {						\
	array_alloc(arrayp,count);			\
	(arrayp)->end = (arrayp)->begin + (count);				\
	memcpy((arrayp)->begin,start,(count) * sizeof(*(arrayp)->begin)); \
    }

#define array_delete_first(arrayp,count)				\
    {									\
	memmove((arrayp)->begin,					\
		(arrayp)->begin + (count),				\
		((arrayp)->end - (arrayp)->begin - (count)) * sizeof(*(arrayp)->begin)); \
	(arrayp)->end -= (count);					\
    }

#define array_eject(arrayp,n)			\
    ( (arrayp)->end -= n )

typedef array(char) char_array;
typedef array(char*) string_array;
