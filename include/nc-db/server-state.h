#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <ev.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "range.h"
#include "stack.h"
#include "array.h"
#endif

typedef struct {
    ev_io * io;
    struct ev_loop * loop;
}
    server_entry;

typedef struct {
    pthread_mutex_t mutex;
    bool active;
    
    compact_array(ev_io*) watchers;
}
    server_state;

typedef void (*io_callback)(struct ev_loop * loop, ev_io * watch, int recieved_events);

inline static bool server_add(server_state * state, struct ev_loop * loop, ev_io * io, int fd, io_callback callback, int ev_flags)
{
    bool ret = false;

    pthread_mutex_lock(&state->mutex);
    if(state->active)
    {
	*compact_array_push(&state->watchers) = io;
	ev_io_init(io,
		   callback,
		   fd,
		   ev_flags);
	ret = true;	
    }    
    pthread_mutex_unlock(&state->mutex);
    
    return ret;
}

inline static bool server_should_run(server_state * state)
{
    bool run;
    
    pthread_mutex_lock(&state->mutex);

    run = state->active;

    pthread_mutex_unlock(&state->mutex);

    return run;
}

inline static bool server_users_up(server_state * state)
{
    bool run;
    pthread_mutex_lock(&state->mutex);
    run = state->active;
    if(run)
	sem_post(&state->users);
    pthread_mutex_unlock(&state->mutex);
    return run;
}

inline static void server_users_down(server_state * state)
{
    sem_wait(&state->users);
}
