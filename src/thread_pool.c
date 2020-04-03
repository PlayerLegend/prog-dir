#include "precompiled.h"

#define FLAT_INCLUDES

#include "thread_pool.h"

typedef struct {
    int count;
    thread_pool_callback callback;
    void * arg;
    void * error_return;
}
    thread_handler;

struct thread_pool {
    thread_handler handler;
    pthread_t master;
};

static void * run_thread(void * arg)
{
    thread_handler * handler = arg;

    assert(NULL != handler);
    assert(NULL != handler->callback);

    enum { CHILD_NO, CHILD_YES, CHILD_ERROR } child_state = CHILD_NO;
    pthread_t child;

    int id = --handler->count;
    
    if(0 < handler->count)
    {
	if( -1 == pthread_create(&child,NULL,run_thread,arg) )
	    child_state = CHILD_ERROR;
	else
	    child_state = CHILD_YES;
    }

    void * ret = handler->callback(id,handler->arg);

    if(child_state == CHILD_YES)
    {
	void * child_return = NULL;
	pthread_join(child,&child_return);
	return child_return != NULL ? child_return : ret;
    }
    else if(child_state == CHILD_ERROR)
    {
	return handler->error_return;
    }
    else
    {
	return ret;
    }
}

void * thread_pool_run(unsigned int count, thread_pool_callback callback, void * arg, void * error_return)
{
    thread_handler handler =
	{
	    .count = count,
	    .callback = callback,
	    .arg = arg,
	    .error_return = error_return,
	};

    assert(NULL != callback);
    assert(NULL != handler.callback);
    assert(count != 0);
    
    return run_thread(&handler);
}

thread_pool * thread_pool_spawn(unsigned int count, thread_pool_callback callback, void * arg, void * error_return)
{
    thread_pool * ret = malloc(sizeof(*ret));
    ret->handler = (thread_handler)
	{
	    .count = count,
	    .callback = callback,
	    .arg = arg,
	    .error_return = error_return,
	};

    assert(NULL != callback);
    assert(NULL != ret->handler.callback);
    assert(count != 0);
    
    if(-1 == pthread_create(&ret->master,NULL,run_thread,&ret->handler))
    {
	free(ret);
	return error_return;
    }

    return ret;
}

void * thread_pool_join(thread_pool * pool)
{
    void * ret;
    pthread_join(pool->master,&ret);
    free(pool);
    return ret;
}

/*void * thread_pool_run(unsigned int count, thread_pool_callback callback, void * arg)
{
    pthread_t * threads = malloc(count * sizeof(*threads));

    void * ret = NULL;

    void * thread_ret;

    unsigned int i;
    
    for( i = 0; i < count; i++ )
	pthread_create( threads + i, NULL, callback, arg );

    for( i = 0; i < count; i++ )
    {
	pthread_join( threads[i], &thread_ret );
	if(thread_ret)
	    ret = thread_ret;
    }

    return ret;
}
*/
