#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <ev.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

#include "range.h"
#include "stack.h"
#include "array.h"
#include "network.h"
#include "tcp_loop.h"
#include "print.h"
#include "queue.h"
#include "thread_pool.h"

typedef struct host_thread host_thread;
typedef struct host_handle host_handle;

struct tcp_reference {
    pthread_mutex_t lock;
    tcp_handle * tcp;
};

struct tcp_handle {
    struct host_thread * thread;
    tcp_callback callback[MAX_TCP_EV];
    tcp_state state;
    ev_io watch;
    char_array read_buffer, write_buffer;
    int fd;
    tcp_custom custom;
    array(tcp_reference*) references;
};

struct host_thread {
    pthread_mutex_t lock;
    pthread_t pthread;
    array(tcp_handle*);
    array(size_t) deleted_index;
    struct ev_loop * loop;
    size_t count;
    host_handle * host;
};

struct host_handle {
    ev_io watch;

    array(host_thread*);
    tcp_callback connect_callback;

    enum
    {
	HOST_HALT = 1 << 2,
	HOST_KEEPALIVE = 1 << 2,
    }
	state;
};
    
#define run_callback(handlep,tcp_event)				\
    {								\
	tcp_callback cb;					\
	if( NULL != (cb = (handlep)->callback[(tcp_event)]) )	\
	    cb(handlep,tcp_event,&(handlep)->custom);			\
    }

#define lock(objectp)				\
    pthread_mutex_lock(&(objectp)->lock)

#define unlock(objectp)				\
    pthread_mutex_unlock(&(objectp)->lock)

#define watch_recall(output,watchp)		\
    (output) = ((typeof(output))((char*)(watchp) - offsetof(typeof(*(output)),watch)))

int tcp_set_keepalive(tcp_handle * handle, bool value)
{
    if( -1 == setsockopt(handle->fd,
			 SOL_SOCKET,
			 SO_KEEPALIVE,
			 &value,
			 sizeof(value)) )
    {
	perror("setsockopt");
	return -1;
    }

    return 0;
}

static host_thread * choose_thread(host_handle * host) // returns the thread with the least number of clients in a locked state
{
    size_t count = -1;
    host_thread * choice = NULL;

    for_range(thread,*host)
    {
	lock(*thread);
	if( (*thread)->count < count )
	{
	    if(choice)
		unlock(choice);
	    choice = *thread;
	    count = choice->count;
	}
	else
	    unlock(*thread);
    }

    return choice;
}

void tcp_set_state(tcp_handle * handle, tcp_state state)
{
    if( handle->state == TCP_STATE_DISCONNECT )
	return;
    
    if( handle->state != state )
    {    
	ev_io_stop(handle->thread->loop,&handle->watch);
	    
	if(state == TCP_STATE_DISCONNECT)
	{
	    run_callback(handle,TCP_EV_DISCONNECT);
	}
	else
	{
	    ev_io_set(&handle->watch,handle->fd,
		      (state & TCP_STATE_READ_ENABLED ? EV_READ : 0) | (state & TCP_STATE_WRITE_ENABLED ? EV_WRITE : 0));
	    ev_io_start(handle->thread->loop,&handle->watch);
	}
	    
	handle->state = state;
    }
}

void tcp_iterate(tcp_handle * handle, tcp_callback callback, void * custom)
{
    for_range(thread,*handle->thread->host)
    {
	if( *thread != handle->thread )
	    lock(*thread);

	for_range(tcp,**thread)
	{
	    handle->custom.call = custom;
	    callback(handle,TCP_EV_CUSTOM,&handle->custom);
	}
	
	if( *thread != handle->thread )
	    unlock(*thread);
    }
}

void listen_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    tcp_handle * tcp;
    watch_recall(tcp,watch);

    lock(tcp->thread);
    
    if( recieved_events & EV_READ )
    {
	run_callback(tcp,TCP_EV_HAVE_MESSAGE);
    }

    if( recieved_events & EV_WRITE )
    {
	run_callback(tcp,TCP_EV_SENT_MESSAGE);
    }

    unlock(tcp->thread);
}

void host_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    assert(recieved_events == EV_READ);
    if( ! (recieved_events & EV_READ) )
	return;

    host_handle * host;
    watch_recall(host,watch);

    int new_fd = tcp_listen(host->watch.fd);
    
    if( -1 == fcntl(new_fd,F_SETFL,fcntl(new_fd,F_GETFL) | O_NONBLOCK) )
    {
	perror("fcntl");
	close(new_fd);
	return;
    }
    
    tcp_handle * tcp = malloc(sizeof(*tcp));
    memset(tcp,0,sizeof(*tcp));

    ev_io_init(&tcp->watch,
	       listen_callback,
	       new_fd,
	       EV_READ | EV_WRITE);

    tcp->state = TCP_STATE_RW;

    host_thread * thread = choose_thread(host);
    tcp->thread = thread;

    ev_io_start(thread->loop,&tcp->watch);

    host->connect_callback(tcp,TCP_EV_CONNECT,&tcp->custom);
    unlock(thread);
}

int tcp_loop_host(const char * service, tcp_callback connect_callback)
{
    host_handle host = { 0 };

    for_range(thread,host)
	pthread_join((*thread)->pthread,NULL);

    
}

void tcp_reference_destroy(tcp_reference * reference)
{
    lock(reference);
    if(reference->tcp)
    {
	lock(reference->tcp->thread);
	for_range(ref,reference->tcp->references)
	{
	    if(*ref == reference)
	    {
		array_flip_del(&reference->tcp->references,ref);
		break;
	    }
	}
    }

    unlock(reference);
    pthread_mutex_destroy(&reference->lock);
    free(reference);
}

tcp_reference * tcp_reference_create(tcp_handle * handle)
{
    tcp_reference * ret = calloc(1,sizeof(*ret));
    pthread_mutex_init(&ret->lock,NULL);
    ret->tcp = handle;
    return ret;
}

tcp_handle * tcp_lock(tcp_reference * reference)
{
    lock(reference);

    if(!reference->tcp)
    {
	unlock(reference);
	pthread_mutex_destroy(&reference->lock);
	free(reference);
	return NULL;
    }

    lock(reference->tcp->thread);

    return reference->tcp;
}

void tcp_unlock(tcp_reference * reference)
{
    unlock(reference->tcp->thread);
    unlock(reference);
}
