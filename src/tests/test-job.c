#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>

#define FLAT_INCLUDES

#include "job.h"

job_return high_job(void * arg)
{
    int * count = arg;
    printf("high: %p %d\n",arg,*count);
    sleep(1);
    *count /= 2;
    return *count > 0 ? JOB_HIGH : JOB_DONE;
}

job_return low_job(void * arg)
{
    int * count = arg;
    printf("low: %p %d\n",arg,*count);
    sleep(1);
    *count /= 2;
    return *count > 0 ? JOB_LOW : JOB_DONE;
}

void * job_thread(void * arg)
{
    job_queue * queue = arg;
    
    do
    {
	printf("Running a job\n");
    }
    while( -1 != job_run(queue) );

    printf("Job queue halted\n");
    
    return NULL;
}

int main()
{
    pthread_t thread;
    printf("Creating queue\n");
    fflush(stdout);
    job_queue * queue = job_queue_create();
    printf("Created queue\n");
    pthread_create(&thread,NULL,job_thread,queue);
    printf("Started exec thread\n");

    int count = 3;
    int arg1[count];
    
    for(intptr_t i = 0; i < count; i++)
    {
	printf("Adding low %zd\n",i);
	arg1[i] = i;
	job_forget(queue,false,low_job,arg1 + i);
    }

    printf("Added low priority jobs\n");
    
    int arg2[count];
    
    for(intptr_t i = 0; i < count; i++)
    {
	printf("Adding high %zd\n",i);
	arg2[i] = i;
	job_forget(queue,true,high_job,arg2 + i);
    }

    printf("Added high priority jobs\n");
    
    sleep(10);

    printf("Stopping queue\n");

    job_queue_stop(queue);

    printf("Stopped queue\n");

    pthread_join(thread,NULL);

    printf("Joined thread\n");

    job_queue_destroy(queue);

    printf("Destroyed job queue, done\n");
    
    return 0;
}
