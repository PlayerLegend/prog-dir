#include "precompiled.h"

#define FLAT_INCLUDES

#include "job.h"
//#include "range.h"
#include "list1.h"

struct job {
    list1_node node;
    job_callback call;
    void * arg;
    bool success;
    sem_t sem;
    bool forget;
    job_queue * queue;
};

struct job_queue {
    list1 jobs;
    list1 parked;
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
    while( NULL != (job = (void*)list1_pop(&queue->jobs)) )
    {
	if(job->forget)
	{
	    free(job);
	}
	else
	{
	    job->success = false;
	    post(job);
	}
    }
	
    unlock(queue);
    broadcast(queue);
    
    assert(queue->stopped);
}

void job_queue_destroy(job_queue * queue)
{
    job * job;
    while( (job = (void*)list1_pop(&queue->parked)) )
	free(job);
    assert(job == NULL);
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->cond);
    free(queue);
}

job * job_create(job_queue * queue, bool priority, job_callback call, void * arg)
{
    job * new;

    lock(queue);

    if(!queue->stopped)
    {
	if( !(new = (void*)list1_pop(&queue->parked)) )
	    new = malloc(sizeof(*new));
	
	sem_init(&new->sem,0,0);
	new->call = call;
	new->arg = arg;
	new->success = false;
	new->forget = false;
	new->queue = queue;

	if(priority)
	    list1_add_next(&queue->jobs,&new->node);
	else
	    list1_add_last(&queue->jobs,&new->node);
    }
    else
    {
	new = NULL;
    }

    unlock(queue);
    signal(queue);

    return new;
}

void job_forget(job_queue * queue, bool priority, job_callback call, void * arg)
{
    job * new;

    lock(queue);

    if(!queue->stopped)
    {
	if( !(new = (void*)list1_pop(&queue->parked)) )
	    new = malloc(sizeof(*new));
	
	new->call = call;
	new->arg = arg;
	new->success = false;
	new->forget = true;
	new->queue = queue;

	if(priority)
	    list1_add_next(&queue->jobs,&new->node);
	else
	    list1_add_last(&queue->jobs,&new->node);
    }

    unlock(queue);
    signal(queue);
}

int job_run(job_queue * queue)
{
    bool priority;
    bool stopped;
    job * job;

    lock(queue);

    while( !(stopped = queue->stopped) && !(job = (void*)list1_pop(&queue->jobs)) )
	wait_queue(queue);
    
    unlock(queue);

    if(stopped)
	return -1;
    list1 * list = NULL;
    bool next = false;

    switch( job->call(job->arg) )
    {
    default:
    case JOB_DONE:
	if(job->forget)
	{
	    list = &queue->parked;
	    next = true;
	}
	else
	{
	    list = NULL;
	    job->success = true;
	    post(job);
	}
	break;
	
    case JOB_LOW:
	list = &queue->jobs;
	next = false;
	break;

    case JOB_HIGH:
	list = &queue->jobs;
	next = true;
	break;
    }

    if(list)
    {
	lock(queue);
	if(next)
	    list1_add_next(list,&job->node);
	else
	    list1_add_last(list,&job->node);
	unlock(queue);
    }
    return 0;
}

int job_wait(void ** arg, job * job)
{
    wait_job(job);

    int ret = job->success ? 0 : -1;
    
    sem_destroy(&job->sem);
    
    if(arg)
	*arg = job->arg;
    lock(job->queue);
    list1_add_next(&job->queue->parked,&job->node);
    unlock(job->queue);
    return ret;
}
