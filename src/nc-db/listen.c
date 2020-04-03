#define FLAT_INCLUDES

#include <stdio.h>
#include <ev.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>

#include "nc-db/server-state.h"
#include "nc-db/listen.h"
#include "nc-db/connection.h"
#include "network.h"

typedef struct {
    ev_io watch;
    server_state * state;
}
    listen_handler;

#define lock(objectp)				\
    pthread_mutex_lock(&(objectp)->mutex)

#define unlock(objectp)				\
    pthread_mutex_unlock(&(objectp)->mutex)

static void listen_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    assert(recieved_events == EV_READ);

    listen_handler * handler = (void*)watch;

    if(!server_should_run(handler->state))
    {
	ev_io_stop(loop,watch);
	close(watch->fd);
	sem_wait(&handler->state->users);
	return;
    }

    int fd = tcp_listen(watch->fd);

    io_connection(loop,fd,handler->state);
}

void io_listen(struct ev_loop * loop, int fd, server_state * state)
{
    if(!server_users_up(state))
	return;
    
    sem_post(&state->users);
    
    listen_handler * new = malloc(sizeof(*new));

    new->state = state;

    ev_io_init(&new->watch,
	       listen_callback,
	       fd,
	       EV_READ);
    
    ev_io_start(loop,&new->watch);
}
