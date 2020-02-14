#define FLAT_INCLUDES
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "job.h"
#include "stack.h"
#include "array.h"
#include "queue.h"

struct job {
    job_callback call;
    void * arg;
    void * retval;
    bool success;
    sem_t sem;
    bool forget;
};

struct job_queue {
    queue(job) jobs;
    bool stopped;
    pthread_cond_t cond;
    pthread_mutex_t lock;
};

#define lock(objectp)				\
    pthread_mutex_lock(&(objectp)->lock)

#define unlock(objectp)				\
    pthread_mutex_unlock(&(objectp)->lock)

#define wait_queue(objectp)				\
    pthread_cond_wait(&(objectp)->cond,&(objectp)->lock)

#define wait_job(objectp)			\
    sem_wait(&(objectp)->sem)

#define post(objectp)			\
    sem_post(&(objectp)->sem)
    
#define signal(objectp)				\
    pthread_cond_signal(&(objectp)->cond)

#define broadcast(objectp)			\
    pthread_cond_broadcast(&(objectp)->cond)

job_queue * job_queue_create()
{
    job_queue * ret = calloc(1,sizeof(*ret));
    pthread_mutex_init(&ret->lock,NULL);
    pthread_cond_init(&ret->cond,NULL);
    return ret;
}

void job_queue_stop(job_queue * queue)
{
    lock(queue);
    queue->stopped = true;
    job * job;
    while( NULL != (job = queue_pop(&queue->jobs)) )
    {
	if(!job->forget)
	{
	    job->success = false;
	    post(job);
	}
    }
    assert(queue->stopped);
    unlock(queue);

    broadcast(queue);
}

void job_queue_destroy(job_queue * queue)
{
    queue_clear(&queue->jobs);
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->cond);
    free(queue);
}

job * job_create(job_queue * queue, job_callback call, void * arg)
{
    job * new;
    lock(queue);
    if(queue->stopped)
    {
	unlock(queue);
	return NULL;
    }
    else
    {
	new = queue_push(&queue->jobs);
	sem_init(&new->sem,0,0);
	new->call = call;
	new->arg = arg;
	new->success = false;
	new->forget = false;
	unlock(queue);
	signal(queue);
	return new;
    }
}

void job_forget(job_queue * queue, job_callback call, void * arg)
{
    job * new;
    lock(queue);
    if(queue->stopped)
    {
	unlock(queue);
    }
    else
    {
	new = queue_push(&queue->jobs);
	new->call = call;
	new->arg = arg;
	new->success = false;
	new->forget = true;
	unlock(queue);
	signal(queue);
    }
}

static unsigned int _job_run(job_queue * queue, unsigned int count)
{
    job * run;
    while(count > 0)
    {
	run = queue_pop(&queue->jobs);
	
	if(!run)
	    return count;

	count--;
	
	run->retval = run->call(run->arg);
	
	if(!run->forget)
	{
	    run->success = true;
	    post(run);
	}
    }

    return 0;
}

int job_run(job_queue * queue)
{
    job * run;
    
    lock(queue);

    while( !queue->stopped && NULL == (run = queue_pop(&queue->jobs)) )
	wait_queue(queue);
    
    if(queue->stopped)
    {
	unlock(queue);
	return -1;
    }
    
    unlock(queue);
    run->retval = run->call(run->arg);
	
    if(!run->forget)
    {
	run->success = true;
	post(run);
    }
    
    return 0;
}

int job_wait(void ** retval, job * job)
{
    int ret;
    wait_job(job);
    if(job->success)
    {
	*retval = job->retval;
	ret = 0;
    }
    else
    {
	ret = -1;
    }
    sem_destroy(&job->sem);
    return ret;
}
