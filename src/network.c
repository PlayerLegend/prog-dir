#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "print.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

static pthread_mutex_t net_lock_mutex = PTHREAD_MUTEX_INITIALIZER;

#define net_lock()				\
    pthread_mutex_lock(&net_lock_mutex)

#define net_unlock()				\
    pthread_mutex_unlock(&net_lock_mutex)

static struct addrinfo * get_info(const char * node, const char * service, int socktype)
{
    struct addrinfo hints = { .ai_family = AF_UNSPEC, .ai_socktype = socktype, .ai_flags = AI_PASSIVE };

    struct addrinfo * got_info;

    int gai_status;

    if( 0 != (gai_status = getaddrinfo(node,service,&hints,&got_info)) )
    {
	print_error("getaddrinfo: %s\n",gai_strerror(gai_status));

	return NULL;
    }

    return got_info;    
}

static int create_socket(struct addrinfo * info)
{
    int fd;

    if( -1 == (fd = socket(info->ai_family, info->ai_socktype, info->ai_protocol)) )
    {
	perror("socket");
	return -1;
    }

    return fd;
}

static int bind_socket(struct addrinfo * info)
{
    int fd;

    if( -1 == (fd = create_socket(info)) )
    {
	return -1;
    }
    
    if( -1 == bind(fd,info->ai_addr,info->ai_addrlen) )
    {
	perror("error binding addrinfo's socket");
	close(fd);
	net_unlock();
	return -1; 
    }

    net_unlock();

    return fd;
}

int tcp_connect(const char * node, const char * service)
{
    struct addrinfo * info;

    if( NULL == (info = get_info(node,service,SOCK_STREAM)) )
    {
	return -1;
    }

    int conn;

    if( -1 == (conn = create_socket(info)) )
    {
	freeaddrinfo(info);
	return -1;
    }


    if( -1 == connect(conn,info->ai_addr,info->ai_addrlen) )
    {
	perror("connect");
	freeaddrinfo(info);
	close(conn);
	return -1;
    }
    
    freeaddrinfo(info);

    assert(conn != -1);
    
    return conn;
}

FILE * tcp_connect_stream(const char * node, const char * service)
{
    int connect;

    if( -1 == (connect = tcp_connect(node,service)) )
    {
	return NULL;
    }

    FILE * ret;

    if( NULL == (ret = fdopen(connect,"r+b")) )
    {
	perror(__func__);
    }

    return ret;
}

static int net_host(char * service, int socktype)
{
    struct addrinfo * info;

    if( NULL == (info = get_info(NULL,service,socktype)) )
	return -1;
    
    int fd = bind_socket(info);
    
    freeaddrinfo(info);

//#ifndef NDEBUG
//    int val = 1;
//    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val));
//#endif

    return fd;
}

int tcp_host(char * service)
{
    return net_host(service,SOCK_STREAM);
}

int udp_host(char * service)
{
    return net_host(service,SOCK_DGRAM);
}

int tcp_listen(int fd)
{
    int ret;

    if( -1 == listen(fd,10) )
    {
	perror("listen");
	return -1;
    }
    
    if( -1 == (ret = accept(fd,NULL,NULL)) )
    {
	perror("accept");
	return -1;
    }

    assert(ret != -1);
    
    return ret;
}

FILE * tcp_listen_stream(int listen)
{
    int connect;

    if( -1 == (connect = tcp_listen(listen)) )
	return NULL;

    FILE * ret;

    if( NULL == (ret = fdopen(connect,"r+b")) )
	perror(__func__);

    assert(ret != NULL);

    return ret;
}
