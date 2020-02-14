#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#include "stack.h"
#include "array.h"
#include "queue.h"
#include "thread_pool.h"

#include <unistd.h>

int callback(thread_job * job, void * global)
{
    printf("job %p waits\n",job);
    sleep(1);
    printf("job %p finishes\n",job);
    return 0;
}

int main()
{
    const int n = 4 * 4;
    thread_pool pool;
    thread_pool_init(&pool,NULL);
    thread_pool_set(&pool,n / 2);
    thread_job jobs[n];

    for(int i = 0; i < n; i++)
    {
	jobs[i] = (thread_job){ .callback = callback };
	thread_job_start(&pool,jobs + i);
    }
    
    for(int i = 0; i < n; i++)
    {
	printf("waiting for job %d\n",i + 1);
	thread_job_wait(jobs + i);
    }

    printf("halting threads\n");
    thread_pool_set(&pool,0);

    printf("done\n");

    return 0;
}
