#include "stack.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define min_alloc (1 << 8)

void*
stack_push(stack * push, ssize_t size)
{
    if( (char*)push->alloc - (char*)push->end <= size )
    {
	size_t alloc = (char*)push->alloc - (char*)push->begin;
	size_t count = (char*)push->end - (char*)push->begin;
	size_t new_alloc;

	if( alloc == 0 )
	{
	    new_alloc = min_alloc;
	}
	else
	{
	    new_alloc = alloc << 1;
	}
        
	push->begin = realloc(push->begin,new_alloc);
	push->end = (char*)push->begin + count;
	push->alloc = (char*)push->begin + new_alloc;
    }
    
    void * ret = push->end;

    push->end = (char*)push->end + size;

    return ret;
}

void
stack_free(stack * dealloc)
{
    free(dealloc->begin);
    dealloc->begin = NULL;
    dealloc->end = NULL;
    dealloc->alloc = NULL;
}

void
stack_forget(stack * reset)
{
    reset->begin = NULL;
    reset->end = NULL;
    reset->alloc = NULL;
}

void*
stack_pop(stack * pop, ssize_t size)
{
    assert(pop->end != pop->begin);
    pop->end = (char*)pop->end - size;
    return pop->end;
}

/* not yet used
void
stack_shift_down(stack * del, ssize_t offset, ssize_t size)
{
    size_t stack_size = (char*)del->end - (char*)del->begin;
    size_t shift_begin = offset + size;
    assert(shift_begin <= stack_size);
    memcpy((char*)del->begin + offset,
	   (char*)del->begin + shift_begin,
	   stack_size - shift_begin);
}
*/

void
stack_copy(stack * dst, stack * src)
{
    size_t src_count = src->end - src->begin;
    size_t src_alloc = src->alloc - src->begin;
    dst->begin = malloc(src_alloc * sizeof(*src->begin));
    dst->end = dst->begin + src_count;
    dst->alloc = dst->begin + src_alloc;
}

void stack_alloc(stack * stack, ssize_t n)
{
    if( n > (char*)stack->alloc - (char*)stack->begin )
    {
	size_t end = (char*)stack->end - (char*)stack->begin;
	stack->begin = realloc(stack->begin,n);
	stack->end = (char*)stack->begin + end;
	stack->alloc = (char*)stack->begin + n;
    }
}
