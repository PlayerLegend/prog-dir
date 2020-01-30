#include "network.h"
#include "print.h"
#include <ev.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include "stack.h"
#include "array.h"
#include "for_range.h"
#include <unistd.h>

struct {
    array(int) array;
    array(int) deleted_index;
}
    client;

void read_fd(int fd, char ** buffer, ssize_t * size)
{
    if( *size == 0 )
    {
	*size = 1 << 10;
	*buffer = malloc(*size + 1);
    }

    struct { char * pos; ssize_t size } get;

    get.pos = *buffer;
    get.size = *size;
    
    int got;

    while( get.size == (got = read(fd,get.pos,get.size)) )
    {
	get.pos = *buffer + *size;
	get.size = *size;
	*size *= 2;
	*buffer = realloc(*buffer,*size + 1);
    }

    *size -= get.size - got;
    (*buffer)[*size] = '\0';
}

void add_client_to_array(int new_fd)
{
    if(!is_range_empty(client.deleted_index))
    {
	int index = array_pop(&client.deleted_index);
	client.array.begin[index] = new_fd;
    }
    else
    {
	*array_push(&client.array) = new_fd;
    }
}

void client_io_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    static struct { char * text; ssize_t len; } message;
    
    if(recieved_events & EV_READ)
    {
	read_fd(watch->fd,&message.text,&message.len);
	printf("message: %s\n",message.text);
    }
    else
    {

    }
}
void listen_io_callback(struct ev_loop * loop, ev_io * watch, int recieved_events)
{
    if(recieved_events & EV_READ)
    {
	int client_fd;

	if( -1 == (client_fd = tcp_listen(watch->fd)) )
	    return;

	add_client_to_array(client_fd);

	ev_io * client_watcher = malloc(sizeof(*client_watcher));
	ev_init(client_watcher,client_io_callback);
	ev_io_set(client_watcher,client_fd,EV_READ);
	ev_io_start(loop,client_watcher);
    }
    else
    {
	return;
    }    
}
int main(int argc, char * argv[])
{
    if( argc < 2 || argc > 3 )
    {
	print_error("usage: %s [port] [optional motd]",argv[0]);
	exit(1);
    }

    const char * service = argv[1];
    const char * motd = argv[2];

    int listen_fd;
    
    if( -1 == (listen_fd = tcp_host(service)) )
    {
	exit(1);
    }

    struct ev_loop * loop = ev_loop_new(EVFLAG_AUTO);
    ev_io listen_watcher;
    ev_init(&listen_watcher,listen_io_callback);
    ev_io_set(&listen_watcher,listen_fd,EV_READ);
    ev_io_start(loop,&listen_watcher);

    ev_run(loop,0);
    
    return 0;
}
