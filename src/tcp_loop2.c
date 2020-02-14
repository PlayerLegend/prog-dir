#define FLAT_INCLUDES

#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ev.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

#include "range.h"
#include "stack.h"
#include "array.h"
#include "thread_pool.h"
#include "tcp_loop2.h"

#define watch_recall(output,watchp)		\
    (output) = ((typeof(output))((char*)(watchp) - offsetof(typeof(*(output)),watch)))

struct tcp_loop {
    compact_array(tcp_connection);
    
    struct ev_loop * ev_loop;
    pthread_mutex_t mutex;

    tcp_connection * master;
    struct tcp_loop * child;

    void * custom;

    sem_t sem;
};

typedef struct {
    array(char);
    size_t point;
}
    buffer;

struct tcp_connection {
    bool active;
    int fd;
    ev_io watch;
    tcp_state state;
    tcp_callback callback[TCP_EV_NONE];
    tcp_loop * loop;
    bool write_toggle;
    buffer to_write[2], to_read;
    void * custom;
};

#define lock(objectp)				\
    pthread_mutex_lock(&(objectp)->mutex)

#define unlock(objectp)				\
    pthread_mutex_unlock(&(objectp)->mutex)

static void connection_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    
}

static tcp_connection * new_connection(tcp_loop * loop, int fd)
{
    tcp_connection * ret = compact_array_push(loop);

    ret->active = true;
    ret->fd = fd;
    ev_io_init(&ret->watch,
	       connection_callback,
	       fd,
	       EV_READ | EV_WRITE);
    ret->state = TCP_STATE_RW;
    ret->loop = loop;

    ev_io_start(loop->ev_loop,&ret->watch);

    return ret;
}

void tcp_set_callback(tcp_connection * con, tcp_event event, tcp_callback callback)
{
    if( event < TCP_EV_NONE )
	con->callback[event] = callback;
}
void tcp_print(tcp_connection * con, const char * fmt, ...);
char_array * tcp_get_write_buffer(tcp_connection * con);
char_range * tcp_get_read_buffer(tcp_connection * con);

void tcp_set_state(tcp_connection * con, tcp_state state);
int tcp_set_keepalive(tcp_connection * con, bool value)
{
    if( -1 == setsockopt(con->fd,
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

void tcp_halt_master(tcp_connection * con)
{
    if(!con->loop->master)
    {
	tcp_set_state(con,TCP_STATE_DISCONNECT);
	return;
    }
    
    tcp_loop * loop = con->loop->master->loop;

    while(loop)
    {
	if(loop != con->loop)
	    lock(loop);

	for_range(child,*loop)
	    tcp_set_state(child,TCP_STATE_DISCONNECT);

	loop = loop->child;

	if(loop != con->loop)
	    unlock(loop);
    }
}

void tcp_iterate(tcp_connection * con, tcp_callback callback)
{
    tcp_loop * loop = con->loop->host->loop;

    while(loop)
    {
	if(loop != con->loop)
	    lock(loop);

	for_range(child,*loop)
	    callback(child,TCP_EV_NONE,loop->custom,&child->custom);

	loop = loop->child;

	if(loop != con->loop)
	    unlock(loop);
    }    
}

tcp_connection * tcp_lock(tcp_reference * reference);
void tcp_unlock(tcp_reference * reference);
