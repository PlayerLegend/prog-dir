#define FLAT_INCLUDES
#include <ev.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "nc-db/server-state.h"
#include "nc-db/connection.h"
//#include "range.h"
#include "stack.h"
#include "array.h"
#include "nc-db/buffer.h"

typedef struct {
    ev_io watch;
    server_state * state;
    char_array read_buffer;
    
}
    connection_handler;

#define lock(objectp)				\
    pthread_mutex_lock(&(objectp)->mutex)

#define unlock(objectp)				\
    pthread_mutex_unlock(&(objectp)->mutex)

static void connection_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    connection_handler * handler = (void*)watch;

    if(!server_should_run(handler->state))
    {
	ev_io_stop(loop,watch);
	close(watch->fd);
	server_users_down(handler->state);
	return;
    }

    if( recieved_events & EV_READ )
    {
	buffer_grow(&handler->read_buffer,watch->fd);
    }
}

void io_connection(struct ev_loop * loop, int fd, server_state * state)
{
    if(!server_users_up(state))
	return;
    
    connection_handler * new = malloc(sizeof(*new));

    new->state = state;

    ev_io_init(&new->watch,
	       connection_callback,
	       fd,
	       EV_READ);
    
    ev_io_start(loop,&new->watch);
}
