#include "precompiled.h"
#include <ev.h>

#define FLAT_INCLUDES

//#include "range.h"
//#include "print.h"
#include "tcp_event.h"
#include "network.h"

typedef struct {
    ev_io watch;
    const tcp_event_config * config;
    bool server_halting;
    size_t client_count;
    pthread_mutex_t lock;
}
    listen_handler;

typedef struct {
    ev_io watch;
    bool active;
}
    io_state;
	
typedef struct {
    io_state write, read;
    char_array read_bytes;
    size_t write_point;
    tcp_event_connection_state state;
    const tcp_event_config * config;
    struct ev_loop * loop;
    listen_handler * listener;
}
    connection_handler;

#define recall_state_from_write_watch(ptr)		\
    ((connection_handler*)((char*)(ptr) - offsetof(connection_handler,write.watch)))

#define recall_state_from_read_watch(ptr)		\
    ((connection_handler*)((char*)(ptr) - offsetof(connection_handler,read.watch)))

#define recall_handler_from_state(ptr)		\
    ((connection_handler*)((char*)(ptr) - offsetof(connection_handler,state)))

static int read_append(char_array * buffer,int fd)
{
    static const size_t get_size = 1 << 8;

    int got_size;

    array_alloc(buffer,buffer->end - buffer->begin + get_size);

    if( 0 >= (got_size = read(fd,buffer->end,get_size)) )
    {
	return -1;
    }

    buffer->end += got_size;

    return 0;
}

inline static void halt_io_state(struct ev_loop * loop, io_state * state)
{
    assert(state->active);
    ev_io_stop(loop,&state->watch);
    state->active = false;
}

inline static void start_io_state(struct ev_loop * loop, io_state * state)
{
    assert(!state->active);
    ev_io_start(loop,&state->watch);
    state->active = true;
}

static void destroy_connection_handler(connection_handler * handler)
{
    handler->config->disconnect(&handler->state);

    printf("halting connection handler\n");
    
    if(handler->state.write.active)
    {
	printf("halted write\n");
	halt_io_state(handler->loop,&handler->write);
    }

    if(handler->state.read.active)
    {
	printf("halted read\n");
	halt_io_state(handler->loop,&handler->read);
    }

    close(handler->read.watch.fd);
    free(handler);
}

static void halt_server(connection_handler * handler)
{
    pthread_mutex_lock(&handler->listener->lock);
    if(!handler->listener->server_halting)
    {
	printf("Halting server\n");
	handler->listener->server_halting = true;
	close(handler->listener->watch.fd);
	ev_io_stop(handler->loop,&handler->listener->watch);
    }
    
    pthread_mutex_unlock(&handler->listener->lock);
}

static bool apply_state_changes(connection_handler * handler)
{
    pthread_mutex_lock(&handler->listener->lock);
    if(handler->listener->server_halting)
	handler->state.disconnect = true;
    pthread_mutex_unlock(&handler->listener->lock);
        
    if(handler->state.halt_server)
    {
	halt_server(handler);
	handler->state.disconnect = true;
    }
    
    if(handler->state.disconnect)
    {
	destroy_connection_handler(handler);
	return false;
    }
    
    if(handler->state.write.active != handler->write.active)
    {
	if(handler->state.write.active)
	{
	    start_io_state(handler->loop,&handler->write);
	}
	else
	{
	    halt_io_state(handler->loop,&handler->write);
	}
    }

    if(handler->state.read.active != handler->read.active)
    {
	if(handler->state.read.active)
	{
	    start_io_state(handler->loop,&handler->read);
	}
	else
	{
	    halt_io_state(handler->loop,&handler->read);
	}
    }

    return true;
}

static void finish_write(struct ev_loop * loop, connection_handler * handler)
{
    halt_io_state(loop,&handler->write);
    handler->state.write.active = false;
    handler->write_point = 0;
    array_rewrite(&handler->state.write.bytes);
    handler->config->finished_write(&handler->state);    
    apply_state_changes(handler);
}

static void write_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    assert(recieved_events & EV_WRITE);

    connection_handler * handler = recall_state_from_write_watch(watch);
    
    if(recieved_events & EV_WRITE)
    {
	size_t total_size = count_range(handler->state.write.bytes);

	if(total_size <= handler->write_point)
	{
	    assert(total_size == handler->write_point);
	    finish_write(loop,handler);
	    return;
	}
	
	ssize_t sent_size = write(watch->fd,
				  
				  handler->state.write.bytes.begin
				  + handler->write_point,

				  total_size
				  - handler->write_point);

	handler->write_point += sent_size;

	if(total_size <= handler->write_point)
	{
	    assert(total_size == handler->write_point);
	    finish_write(loop,handler);
	    return;
	}
    }
}

static char * find_term(char * begin, char * end, char_array * terms)
{
    for(char * byte = begin; byte < end; byte++)
    {
	for_range(term,*terms)
	{
	    if(*byte == *term)
		return byte;
	}
    }

    return NULL;
}

static void read_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    assert(recieved_events & EV_READ);

    connection_handler * handler = recall_state_from_read_watch(watch);
    
    if(recieved_events & EV_READ)
    {
	if( -1 == read_append(&handler->read_bytes,watch->fd) )
	{
	    destroy_connection_handler(handler);
	    return;
	}
    }

    char * term;
    char * finished = handler->read_bytes.begin;
    size_t count;

    assert(handler != NULL);
    
    while(handler->state.read.active && NULL != (term = find_term(finished,handler->read_bytes.end,&handler->state.read.term_bytes)))
    {
	count = term - finished + 1;
	array_write_several(&handler->state.read.bytes,
			    finished,
			    count);
	finished += count;
	handler->config->finished_read(&handler->state);
	if( !apply_state_changes(handler) )
	    return;
    }

    while(handler->state.read.active && handler->state.read.term_size < (size_t)(handler->read_bytes.end - finished))
    {
	count = handler->state.read.term_size;
	array_write_several(&handler->state.read.bytes,
			    finished,
			    count);
	finished += count;
	
	handler->config->finished_read(&handler->state);
	if( !apply_state_changes(handler) )
	    return;
    }

    if(finished != handler->read_bytes.begin)
    {
	count = finished - handler->read_bytes.begin;
	array_delete_first(&handler->read_bytes,count);
    }
}

static void create_connection_handler(int connection_fd, struct ev_loop * loop, listen_handler * listen)
{
    connection_handler * handler = malloc(sizeof(*handler));
    *handler = (connection_handler)
    {
	.loop = loop,
	.config = listen->config,
	.state.read.active = true,
	.state.custom.server = listen->config->custom,
	.listener = listen,
    };

    ev_io_init(&handler->write.watch,
	       write_callback,
	       connection_fd,
	       EV_WRITE);
    
    ev_io_init(&handler->read.watch,
	       read_callback,
	       connection_fd,
	       EV_READ);

    listen->config->connect(&handler->state);
    apply_state_changes(handler);
}

void listen_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    listen_handler * listen_handler = (void*)watch;
    
    if(recieved_events & EV_READ)
    {
	pthread_mutex_lock(&listen_handler->lock);
	int client_fd = tcp_listen(watch->fd);
	
	if( -1 == fcntl(client_fd,F_SETFL,fcntl(client_fd,F_GETFL) | O_NONBLOCK) )
	{
	    perror("fcntl");
	    close(client_fd);
	    return;
	}

	if(listen_handler->config->keepalive)
	{
	    int setsockopt_true = 1;
	    
	    if( -1 == setsockopt(client_fd,
				 SOL_SOCKET,
				 SO_KEEPALIVE,
				 &setsockopt_true,
				 sizeof(setsockopt_true)) )
	    {
		perror("setsockopt");
		close(client_fd);
		pthread_mutex_unlock(&listen_handler->lock);
		return;
	    }
	}

	listen_handler->client_count++;
	create_connection_handler(client_fd,loop,listen_handler);
	pthread_mutex_unlock(&listen_handler->lock);
    }
}

int tcp_event_listen(const char * service, const tcp_event_config * config)
{
    int listen_fd;

    if( -1 == (listen_fd = tcp_host(service)) )
    {
	return -1;
    }

    struct ev_loop * loop = ev_loop_new(EVFLAG_AUTO);

    listen_handler listen_handler = { .config = config };
    pthread_mutex_init(&listen_handler.lock,NULL);

    ev_io_init(&listen_handler.watch,listen_callback,listen_fd,EV_READ);
    ev_io_start(loop,&listen_handler.watch);

    ev_run(loop,0);

    return 0;
}

int tcp_event_connect(const char * host, const char * service, const tcp_event_config * config)
{
    int connect_fd;

    if( -1 == (connect_fd = tcp_connect(host,service)) )
    {
	return -1;
    }

    struct ev_loop * loop = ev_loop_new(EVFLAG_AUTO);

    
}

void tcp_event_wake_write(tcp_event_connection_state * state)
{
    connection_handler * handler = recall_handler_from_state(state);
    handler->state.write.active = true;
    apply_state_changes(handler);
}
