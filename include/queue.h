#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#include "stack.h"
#include <string.h>
#include "array.h"
#include <stdbool.h>
#endif

#define queue(type)				\
    struct {					\
	array(type) list[2];			\
	ssize_t index;				\
	bool pop;				\
    }

typedef queue(char) char_queue;

void * _queue_pop(char_queue * queue, size_t element_size);
void * _queue_push(char_queue * queue, size_t element_size);

#define queue_pop(qp)				\
    ( (typeof((qp)->list[0].begin)) _queue_pop( (char_queue*)(qp) , sizeof(*(qp)->list[0].begin) ) )

#define queue_push(qp)				\
    ( (typeof((qp)->list[0].begin)) _queue_push( (char_queue*)(qp) , sizeof(*(qp)->list[0].begin) ) )
