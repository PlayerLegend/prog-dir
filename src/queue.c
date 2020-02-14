#include "queue.h"
#include "range.h"

void * _queue_pop(char_queue * queue, size_t element_size)
{
    /*printf("element size %zu, pop %d, index %zu, size %zd + %zd\n",
	   element_size,
	   queue->pop,
	   queue->index,
	   count_range(queue->list[0]) / element_size,
	   count_range(queue->list[1]) / element_size);*/
    char_array * list = (void*)(queue->list + queue->pop);
    if( (size_t)count_range(*list) <= queue->index * element_size )
    {
	array_rewrite(list);
	queue->pop = !queue->pop;
	queue->index = 0;
	list = (void*)(queue->list + queue->pop);
	if(is_range_empty(*list))
	{
	    //printf("queue empty\n");
	    return NULL;
	}
    }

    return list->begin + element_size * (queue->index++);
}

void * _queue_push(char_queue * queue, size_t element_size)
{
    //printf("push\n");
    return stack_push( (void*)(queue->list + !queue->pop), element_size );    
}
