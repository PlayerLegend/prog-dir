#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#include "stack.h"
#include <string.h>
#endif

#define array(type)				\
    union {					\
	stack stack;				\
	struct {				\
	    type * begin;			\
	    type * end;				\
	};					\
    }

#define array_count(arrayp)			\
    ((arrayp)->end - (arrayp)->begin)

#define array_rewrite(arrayp)			\
    stack_rewrite(&(arrayp)->stack)

#define array_forget(arrayp)			\
    stack_forget(&(arrayp)->stack)

#define array_indexof(arrayp,valuep)		\
    ((valuep) - (arrayp)->begin)

#define array_push(arrayp)		\
    (stack_push_type(&(arrayp)->stack,typeof(*(arrayp)->begin)))

#define array_pop(arrayp)		\
    (stack_pop_type(&(arrayp)->stack,typeof(*(arrayp)->begin)))

#define array_flip_del(arrayp,ptr)				\
    *(ptr) = *array_pop(arrayp);

#define  array_rewrite(arrayp)			\
    stack_rewrite(&(arrayp)->stack)

#define array_import_alloc(arrayp,abegin,aend)	\
    stack_import_alloc(&(arrayp)->stack,abegin,aend)

#define array_copy(dstp,srcp)			\
    stack_copy(&(dstp)->stack,&(srcp)->stack)

#define array_append_several(arrayp,start,count)			\
    {									\
	stack_push(&(arrayp)->stack,sizeof(*(arrayp)->begin) * (count)); \
	memcpy((arrayp)->end - (count),start,(count) * sizeof(*(arrayp)->begin)); \
    }
    
#define array_alloc(arrayp,n)			\
    stack_alloc(&(arrayp)->stack,(n) * (sizeof(*(arrayp)->begin)))

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
