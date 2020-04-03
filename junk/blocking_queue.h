#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#define FLAT_INCLUDES

#include "stack.h"
#include "array.h"
#include "queue.h"
#endif

#define blocking_queue(type)			\
    struct { queue(type); bool stopped; pthread_cond_t cond; pthread_mutex_t mutex; }

typedef blocking_queue(char) char_blocking_queue;

#define blocking_queue_init(qp)			\
    {						\
	memset((qp),0,sizeof(*(qp)));		\
	pthread_mutex_init(&(qp)->mutex,NULL);	\
	pthread_cond_init(&(qp)->mutex,NULL);	\
    }


#define blocking_queue_stop(qp)			\
    {						\
	pthread_mutex_lock(&(qp)->lock);	\
	(qp)->stopped = true;			\
	pthread_mutex_unlock(&(qp)->lock);	\
	pthread_cond_broadcast(&(qp)->cond);	\
    }

    
    

void * _blocking_queue_pop(char_blocking_queue * queue, size_t element_size);
#define blocking_queue_pop(qp)						\
    ( (typeof((qp)->begin)) _blocking_queue_pop((qp),sizeof( *(qp)->begin )) )
#define blocking_queue_flush(qp)		\
    queue_pop(qp)

#define blocking_queue_clear(qp)			\
    queue_clear(qp)

int _blocking_queue_push(char_blocking_queue * queue, size_t element_size, void * item);
#define blocking_queue_push(qp,item)		\
    _blocking_queue_push( (queue), sizeof(*(qp)->begin), item )
