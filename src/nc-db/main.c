#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <ev.h>
#include <pthread.h>
#include <stdlib.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "nc-db/configuration.h"
#include "nc-db/database.h"
#include "thread_pool.h"

#define lock(objectp)				\
    pthread_mutex_lock(&(objectp)->mutex);

#define unlock(objectp)				\
    pthread_mutex_unlock(&(objectp)->mutex);

typedef struct {
    
    pthread_mutex_t mutex;
    struct ev_loop * loop;
}
    thread_state;

typedef struct {
    pthread_mutex_t mutex;
    array(thread_state*);
    thread_state * thread;
}
    pool_state;

typedef array(struct ev_loop*) loop_array;

typedef enum { IO_HALT, IO_PAUSE, IO_READ, IO_WRITE, IO_RW } io_state;

void io_state_set(struct ev_loop * loop, ev_io * io, io_state state)
{
    
}

void * run_loop(unsigned int id, void * arg)
{
    pool_state * pool = arg;
    ev_run(pool->thread[id].loop,0);
    return NULL;
}

void run_pool(unsigned int count)
{
    pool_state pool = { .thread = malloc(count * sizeof(*pool.thread)) };
    
    for(thread_state *thread = pool.thread, *end = thread + count; thread < end; thread++)
    {
	pthread_mutex_init(&thread->mutex,NULL);
	thread->loop = ev_loop_new(0);
    }
        
    thread_pool_run(count,run_loop,&pool,(void*)1);
    
    for(thread_state *thread = pool.thread, *end = thread + count; thread < end; thread++)
    {
	pthread_mutex_destroy(&thread->mutex);
    }

    free(pool.thread);
}

int main(int argc, char * argv[])
{
    if( -1 == init_config(argc,argv) )
	return -1;

    printf("initialization done, starting network\n");

    if( -1 == network_listen() )
    {
	return 1;
    }

    return 0;
}
