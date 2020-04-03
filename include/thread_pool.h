#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

typedef struct thread_pool thread_pool;
typedef void * (*thread_pool_callback)(unsigned int index, void * arg);

void * thread_pool_run(unsigned int count, thread_pool_callback callback, void * arg, void * error_return);
thread_pool * thread_pool_spawn(unsigned int count, thread_pool_callback callback, void * arg, void * error_return);
void * thread_pool_join(thread_pool * pool);
