#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#include "range.h"
#endif

typedef struct
{
    range(void);
    void * alloc;
}
    stack;

#define stack_not_empty(stackp) \
    ((stackp)->begin != (stackp)->end)

void*
stack_push(stack * push, ssize_t size);

void
stack_free(stack * dealloc);

void
stack_forget(stack * reset);

#define stack_rewrite(stackp)			\
    ((stackp)->end = (stackp)->begin)

void*
stack_pop(stack * pop, ssize_t size);

void
stack_copy(stack * dst, stack * src);

#define stack_push_type(stackp,type)		\
    ((type *)stack_push(stackp,sizeof(type)))

#define stack_pop_type(stackp,type)		\
    ((type*)stack_pop(stackp,sizeof(type)))

#define stack_import_alloc(stackp,abegin,aend)				\
    (*(stackp) = (stack){ .begin = abegin, .end = abegin, .alloc = aend })

void stack_alloc(stack * stack, ssize_t n);
