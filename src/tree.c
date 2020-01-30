#include "stack.h"
#include "tree.h"
#include <stddef.h>

tree_node*
tree_add(tree * add)
{
    void * begin = add->begin;
    void * end = add->end;

    void * ret = stack_push(&add->stack);

    if( add->begin != begin )
    {
	size_t shift = (char*)add->begin - (char*)begin;
	for( tree_node * i = add->begin; i < add->end; i++ )
	{
	    if( i->peer >= begin && i->peer < end )
		i->peer = (char*)i->peer + shift;
	    
	    if( i->child >= begin && i->child < end )
		i->child = (char*)i->child + shift;
	}
    }

    return ret;
}
