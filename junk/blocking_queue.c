
#define FLAT_INCLUDES
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#include "stack.h"
#include "array.h"
#include "queue.h"
#include "blocking_queue.h"

void * _blocking_queue_pop(char_blocking_queue * queue, size_t element_size)
{
    void * ret = NULL;

    pthread_mutex_lock(&queue->mutex);

    while( !queue->stopped && NULL == (ret = _queue_pop((void*)queue,element_size)) )
	pthread_cond_wait(&queue->cond,&queue->mutex);

    pthread_mutex_unlock(&queue->mutex);

    return ret;
}

int _blocking_queue_push(char_blocking_queue * queue, size_t element_size, void * item)
{
    pthread_mutex_lock(&queue->mutex);
    
    if( queue->stopped )
    {
	pthread_mutex_unlock(&queue->mutex);
	return -1;
    }

    memcpy(_queue_push((void*)queue,element_size),item,element_size);
    
    pthread_mutex_unlock(&queue->mutex);

    pthread_cond_signal(&queue->cond);

    return 0;
}
