#define FLAT_INCLUDES
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>

#include "job.h"

void * print_job(void * arg)
{
    printf("job: %zd\n",(intptr_t)arg);
    return NULL;
}

void * job_thread(void * arg)
{
    job_queue * queue = arg;
    
    do
    {
	printf("Running a job");
    }
    while( -1 != job_run(queue) );

    printf("Job queue halted\n");

    job_queue_destroy(queue);
    
    return NULL;
}

int main()
{
    pthread_t thread;
    job_queue * queue = job_queue_create();
    pthread_create(&thread,NULL,job_thread,queue);

    for(intptr_t i = 0; i < 11; i++)
    {
	printf("Adding %zd\n",i);
	job_create(queue,print_job,(void*)i);
    }

    sleep(2);

    job_queue_stop(queue);

    pthread_join(thread,NULL);

    return 0;
}
