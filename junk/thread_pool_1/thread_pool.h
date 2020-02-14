#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#include "stack.h"
#include "array.h"
#include "queue.h"

#endif

typedef struct thread_job {
    int (*callback)(struct thread_job * job, void * global);
    sem_t wait;
    int exit;
    enum { JOB_INCOMPLETE, JOB_DONE, JOB_CANCELLED } state;
}
    thread_job;
    
typedef struct {
    array(pthread_t) halted_threads;
    queue(thread_job*) jobs;
    struct {
	size_t current, want;
    }
	size;
    
    sem_t job_wait, halt_wait;
    pthread_mutex_t lock;
    void * global;
}
    thread_pool;

void thread_pool_init(thread_pool *pool, void *global);
void thread_pool_set(thread_pool *pool, ssize_t n);
void thread_pool_destroy(thread_pool *pool);
void thread_job_start(thread_pool *pool,thread_job *job);
#define thread_job_wait(jobp)			\
    { if( -1 == sem_wait(&(jobp)->wait) ) perror("thread_job_wait"); }

#define thread_job_trywait(jobp)			\
    { if( -1 == sem_trywait(&(jobp)->wait) ) perror("thread_job_wait"); }
