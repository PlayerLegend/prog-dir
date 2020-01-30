#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#include "stack.h"
#include "array.h"
#include <stdbool.h>
#endif

typedef struct {
    struct {
	void * client;
	void * server;
    }
	custom;
    
    struct {
	bool active;
	size_t term_size;
	char_array term_bytes;
	char_array bytes;
    }
	read;

    struct {
	bool active;
	char_array bytes;
    }
	write;
}
    tcp_event_connection_state;

typedef struct {
    void * custom;
    bool keepalive;
    bool (*connect)(tcp_event_connection_state * state);
    bool (*finished_read)(tcp_event_connection_state * state);
    bool (*finished_write)(tcp_event_connection_state * state);
    void (*disconnect)(tcp_event_connection_state * state);
}
    tcp_event_config;

int tcp_event_listen(const char * service, const tcp_event_config * config);
void tcp_event_wake_write(tcp_event_connection_state * state);
