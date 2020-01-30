#define FLAT_INCLUDES
#include <stdio.h>
#include "stack.h"
#include <string.h>
#include "array.h"
#include <stdbool.h>
#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include "thread_pool.h"
#include "for_range.h"
#include <stdlib.h>
#include "print.h"

typedef struct {
    pthread_t thread;
    thread_pool * pool;
}
    thread_handler;

void * pool_thread_func(void * handler_v)
{
    thread_handler * handler = handler_v;
    thread_pool * pool = handler->pool;
    thread_job * job = NULL;
    bool should_run = true;

WAIT:
    sem_wait(&pool->job_wait);
    pthread_mutex_lock(&pool->lock);

    if(pool->size.current > pool->size.want)
    {
	pool->size.current--;
	should_run = false;
    }
    else
    {
	job = *queue_pop(&pool->jobs);
    }
    
    pthread_mutex_unlock(&pool->lock);

    if(!should_run)
	goto EXIT;
    
    if(!job)
	goto WAIT;

    printf("thread %p runs job %p\n",handler,job);
    
    job->exit = job->callback(job,pool->global);
    job->done = true;
    sem_post(&job->wait);

    goto WAIT;

EXIT:
    pthread_mutex_lock(&pool->lock);
    *array_push(&pool->halted_threads) = handler->thread;
    free(handler);
    pthread_mutex_unlock(&pool->lock);

    sem_post(&pool->halt_wait);

    return NULL;
}

static int spawn_thread(thread_pool * pool)
{
    thread_handler * handler = malloc(sizeof(*handler));
    *handler = (thread_handler){ .pool = pool };
    
    if( -1 == pthread_create(&handler->thread,NULL,pool_thread_func,handler) )
    {
	print_error("Failed to spawn a thread");
	free(handler);
	return -1;
    }

    pool->size.current++;

    return 0;
}

static void join_thread(thread_pool * pool)
{
    pthread_mutex_unlock(&pool->lock);
    
    sem_post(&pool->job_wait);
    sem_wait(&pool->halt_wait);
    
    pthread_mutex_lock(&pool->lock);
    for_range(thread,pool->halted_threads)
    {
	pthread_join(*thread,NULL);
    }
    array_rewrite(&pool->halted_threads);
}

void thread_pool_set(thread_pool *pool, ssize_t n)
{
    pthread_mutex_lock(&pool->lock);
    pool->size.want = n;
    while(pool->size.current < pool->size.want)
    {
	if( -1 == spawn_thread(pool) )
	    break;
    }
    
    while(pool->size.current > pool->size.want)
    {
	join_thread(pool);
    }
    
    pthread_mutex_unlock(&pool->lock);
}

void thread_pool_init(thread_pool *pool, void *global)
{
    *pool = (thread_pool){ .global = global };
    pthread_mutex_init(&pool->lock,NULL);
    sem_init(&pool->job_wait,0,0);
    sem_init(&pool->halt_wait,0,0);
}

void thread_job_start(thread_pool *pool,thread_job *job)
{
    job->done = false;
    sem_init(&job->wait,0,0);
    pthread_mutex_lock(&pool->lock);
    *queue_push(&pool->jobs) = job;
    pthread_mutex_unlock(&pool->lock);
    sem_post(&pool->job_wait);
}
