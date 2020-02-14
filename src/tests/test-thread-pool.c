#define FLAT_INCLUDES
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include "thread_pool.h"
#include "job.h"

static void * cb_thread(void * arg)
{
    job_queue * queue = arg;

    printf("Started thread\n");

    while( -1 != job_run(queue) )
    {
	printf("ran a job\n");
    }

    return NULL;
}

static void * cb_job(void * arg)
{
    printf("Sleeping for %zu seconds\n",(uintptr_t)arg);
    sleep((uintptr_t)arg);
    printf("Finished after %zu seconds\n",(uintptr_t)arg);
    return NULL;
}

int main()
{
    struct { int seconds, threads, jobs; } count = { 3, 4, 100 };
    
    job_queue * queue = job_queue_create();

    for(int i = 0; i < count.jobs; i++)
	job_forget(queue,cb_job,(void*)1);

    thread_pool * pool = thread_pool_spawn(count.threads,cb_thread,queue,(void*)1);

    sleep(count.seconds + 1);

    printf("  === stopping job queue ===\n");
    
    job_queue_stop(queue);

    printf("  === attempting to join ===\n");

    assert(NULL == thread_pool_join(pool));

    job_queue_destroy(queue);

    return 0;
}
