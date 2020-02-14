#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "range.h"
#include "stack.h"
#include "array.h"

#endif

typedef struct tcp_connection tcp_connection;
typedef struct tcp_reference tcp_reference;
typedef struct tcp_loop tcp_loop;

typedef enum { TCP_EV_DISCONNECT, TCP_EV_READ, TCP_EV_WROTE, TCP_EV_NONE, TCP_EV_CONNECT } tcp_event;
typedef enum { TCP_STATE_DISCONNECT, TCP_STATE_READONLY, TCP_STATE_WRITEONLY, TCP_STATE_RW } tcp_state;

typedef void (*tcp_callback)(tcp_connection * con, tcp_event event, void * server_custom, void ** connection_custom);

//    connection communication
void tcp_print(tcp_connection * con, const char * fmt, ...);
char_array * tcp_get_write_buffer(tcp_connection * con);
char_range * tcp_get_read_buffer(tcp_connection * con);

//    connection configuration
void tcp_set_callback(tcp_connection * con, tcp_event event, tcp_callback callback);
void tcp_set_state(tcp_connection * con, tcp_state state);
int tcp_set_keepalive(tcp_connection * con, bool value);
void tcp_halt_master(tcp_connection * con);

//    server exploration
void tcp_iterate(tcp_connection * con, tcp_callback callback);
tcp_connection * tcp_lock(tcp_reference * reference);
void tcp_unlock(tcp_reference * reference);
tcp_loop * tcp_get_host_loop(tcp_connection * con);
tcp_loop * tcp_get_master_loop(tcp_connection * con);

//    spawning
tcp_connection * tcp_connection_new(tcp_loop * loop, int fd, tcp_callback connect_callback);
tcp_loop * tcp_loop_create(tcp_loop * parent);

//    running
void tcp_loop_run(tcp_loop * loop);
