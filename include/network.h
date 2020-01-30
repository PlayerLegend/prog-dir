#include <stdio.h>

int tcp_host(const char * service);
int udp_host(const char * service);

int tcp_connect(const char * node, const char * service);
int tcp_listen(int fd);

FILE * tcp_listen_stream(int fd);
FILE * tcp_connect_stream(const char * node, const char * service);
