#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "tcp_event.h"
#include "range.h"
#include "print.h"
#include "print_array.h"
#include "delimit.h"
#include "nc-db/database.h"
#include "nc-db/configuration.h"

typedef struct handler {
    db_handle handle;
    void (*think)(struct handler * handler, tcp_event_connection_state * state, char * message);
}
    handler;

#define client_error(fmt,...)		\
    {						\
	print_error(fmt,##__VA_ARGS__);		\
	state->disconnect = true;		\
	return;					\
    }

void th_add(handler * handler, tcp_event_connection_state * state, char * message)
{
    if( -1 == db_add(handler->handle,message) )
	client_error("Failed to add '%s'\n",message);
}

void th_delete(handler * handler, tcp_event_connection_state * state, char * message)
{
    if( -1 == db_delete(handler->handle,message) )
	client_error("Failed to delete '%s'\n",message);
}

void th_get_values(handler * handler, tcp_event_connection_state * state, char * message)
{
    db_get_values(&state->write.bytes,handler->handle,message);
    state->write.active = true;
}

void th_get_keys(handler * handler, tcp_event_connection_state * state, char * message)
{
    db_get_keys(&state->write.bytes,handler->handle,message);
    state->write.active = true;
}

void th_opener(handler * handler, tcp_event_connection_state * state, char * message)
{
    if( 0 == strcmp(message,"quit") )
    {
	state->halt_server = true;
	return;
    }
    
    clause_config clause_config = { .separator_list = " " };
    clause clause;

    delimit_clause(&clause,&clause_config,message);

    if( -1 == db_load(&handler->handle,clause.predicate) )
	client_error("Failed to open '%s'\n",clause.predicate);
    
    if( 0 == strcmp(clause.subject,"add") )
    {
	handler->think = th_add;
	return;
    }
    
    if( 0 == strcmp(clause.subject,"delete") )
    {
	handler->think = th_delete;
	return;
    }
    
    if( 0 == strcmp(clause.subject,"get-keys") )
    {
	handler->think = th_get_keys;
	return;
    }
    
    if( 0 == strcmp(clause.subject,"get-values") )
    {
	handler->think = th_get_values;
	return;
    }
    
    printf("Unknown command '%s'",clause.subject);
    state->disconnect = true;
    return;
}

static void cb_connect(tcp_event_connection_state * state)
{
    *array_push(&state->read.term_bytes) = '\n';
    state->custom.client = malloc(sizeof(handler));    
    handler * set = state->custom.client;
    *set = (handler){ .think = th_opener };
}

static void terminate(char_array * array)
{
    for_range(byte,*array)
    {
	if(*byte == '\0')
	    return;
	
	if(*byte == '\n')
	{
	    *byte = '\0';
	    return;
	}
    }

    *array_push(array) = '\0';
}

static void cb_finished_read(tcp_event_connection_state * state)
{
    handler * handler = state->custom.client;

    terminate(&state->read.bytes);
    
    handler->think(handler,state,state->read.bytes.begin);
}

static void cb_finished_write(tcp_event_connection_state * state)
{
}

static void cb_disconnect(tcp_event_connection_state * state)
{
    handler * handler = state->custom.client;
    
    db_dump(handler->handle);

    printf("Disconnected client\n");
}

int network_listen()
{
    tcp_event_config event_config =
	{
	    .connect = cb_connect,
	    .finished_read = cb_finished_read,
	    .finished_write = cb_finished_write,
	    .disconnect = cb_disconnect,
	};

    const char * port = config_string(CONFIG_PORT);
    printf("Listening on %s\n",port);
    
    return tcp_event_listen(port,&event_config);
}
