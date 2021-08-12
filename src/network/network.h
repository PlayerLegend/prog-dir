#ifndef FLAT_INCLUDES
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#endif

int tcp_host(const char * service);

int tcp_connect(const char * node, const char * service);
int tcp_listen(int fd);

FILE * tcp_listen_stream(int fd);
FILE * tcp_connect_stream(const char * node, const char * service);

int udp_host(const char * service);

keyargs_declare(int, udp_recv, int fd; buffer_char *message, *host, *service;);
#define udp_recv(...) keyargs_call(udp_recv, __VA_ARGS__);

keyargs_declare(int, udp_send, int fd; range_const_char *message; const char *host; const char *service;);
#define udp_send(...) keyargs_call(udp_send, __VA_ARGS__);
