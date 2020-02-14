#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

typedef void * (*job_callback)(void * arg);
typedef struct job_queue job_queue;
typedef struct job job;

job_queue * job_queue_create();
void job_queue_stop(job_queue * queue);
void job_queue_destroy(job_queue * queue);
job * job_create(job_queue * queue, job_callback call, void * arg);
void job_forget(job_queue * queue, job_callback call, void * arg);
int job_run(job_queue * queue);
int job_wait(void ** retval, job * job);
