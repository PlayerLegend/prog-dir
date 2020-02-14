#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "range.h"
#include "stack.h"
#include "array.h"
#endif

typedef struct tcp_handle tcp_handle;
typedef enum
{
    TCP_EV_CONNECT,
    TCP_EV_DISCONNECT,
    TCP_EV_HAVE_MESSAGE,
    TCP_EV_SENT_MESSAGE,
    TCP_EV_CUSTOM,
    MAX_TCP_EV,
}
    tcp_event;

typedef enum
{
    TCP_STATE_DISCONNECT = 0,
    TCP_STATE_READ_ENABLED = 1 << 1,
    TCP_STATE_WRITE_ENABLED = 1 << 2,
    TCP_STATE_R = TCP_STATE_READ_ENABLED,
    TCP_STATE_W = TCP_STATE_WRITE_ENABLED,
    TCP_STATE_RW = TCP_STATE_READ_ENABLED | TCP_STATE_WRITE_ENABLED,
}
    tcp_state;

typedef struct {
    void *server, *connection, *call;
}
    tcp_custom;

typedef struct tcp_reference tcp_reference;	       

typedef void (*tcp_callback)(tcp_handle * handle, tcp_event event, tcp_custom * custom);

// used by callbacks
int tcp_close(tcp_handle * handle);
int tcp_loop_halt(tcp_handle * handle);
int tcp_loop_host(const char * service, tcp_callback connect_callback);
int tcp_loop_connect(const char * host, const char * service, tcp_callback connect_callback);

int tcp_print(tcp_handle * handle, const char * fmt, ...);
int tcp_sendbytes(tcp_handle * handle, char_array * bytes);
int tcp_recieve(char_array * bytes, tcp_handle * handle, char_range term_bytes, size_t term_length);

int tcp_set_callback(tcp_handle * handle, tcp_event event, tcp_callback callback);
void tcp_set_state(tcp_handle * handle, tcp_state state);
int tcp_set_keepalive(tcp_handle * handle, bool value);

void tcp_iterate(tcp_handle * handle, tcp_callback callback, void * custom);

// used externally
tcp_reference * tcp_reference_create(tcp_handle * handle);
void tcp_reference_destroy(tcp_reference * reference);
tcp_handle * tcp_lock(tcp_reference * reference);
void tcp_unlock(tcp_reference * reference);
