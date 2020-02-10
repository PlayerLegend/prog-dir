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
    union {
	db_single * single;
	db_multiple * multiple;
    };
    void (*think)(struct handler * handler, tcp_event_connection_state * state, char * message);
}
    handler;

#define client_error(fmt,...)		\
    {						\
	print_error(fmt,##__VA_ARGS__);		\
	state->disconnect = true;		\
	return;					\
    }

#define client_write(fmt,...)		\
    {						\
	print_array_append(&state->write.bytes,fmt "\n",##__VA_ARGS__);	\
	state->write.active = true;					\
    }


#define client_kv()				\
    key_value kv;				\
    if( -1 == db_make_kv(&kv,message) )		\
    {						\
	state->disconnect = true;		\
	return;					\
    }

void th_add_multiple(handler * handler, tcp_event_connection_state * state, char * message)
{
    client_kv();
    db_add_multiple(handler->multiple,kv);
}

void th_fwd_multiple(handler * handler, tcp_event_connection_state * state, char * message)
{
    client_write("TODO");
}

void th_rev_multiple(handler * handler, tcp_event_connection_state * state, char * message)
{
    
}

void th_add_single(handler * handler, tcp_event_connection_state * state, char * message)
{
    client_kv();
    db_add_single(handler->single,kv);
}

void th_fwd_single(handler * handler, tcp_event_connection_state * state, char * message)
{
    
}

void th_rev_single(handler * handler, tcp_event_connection_state * state, char * message)
{
    
}

void th_opener(handler * handler, tcp_event_connection_state * state, char * message)
{
    clause_config clause_config = { .separator_list = " " };
    clause clause;
    delimit_clause(&clause,&clause_config,message);

    if( 0 == strcmp(clause.subject,"set-multiple") )
    {
	handler->think = th_add_multiple;
	goto MULTIPLE;
    }

    if( 0 == strcmp(clause.subject,"set-single") )
    {
	handler->think = th_add_single;
	goto SINGLE;
    }

    if( 0 == strcmp(clause.subject,"fwd-multiple") )
    {
	handler->think = th_fwd_multiple;
	goto MULTIPLE;
    }

    if( 0 == strcmp(clause.subject,"get-single") )
    {
	handler->think = th_fwd_single;
	goto SINGLE;
    }

    printf("Unknown command '%s'",clause.subject);

    goto ERROR;

SINGLE:
    if( -1 == db_load_single(&handler->single,clause.predicate) )
    {
	print_error("Failed loading single database at '%s'",clause.predicate);
	goto ERROR;
    }

    return;
    
MULTIPLE:
    if( -1 == db_load_multiple(&handler->multiple,clause.predicate) )
    {
	print_error("Failed loading multiple database at '%s'",clause.predicate);
	goto ERROR;
    }

    return;
    
ERROR:
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

    printf("MESSAGE: %s\n",state->read.bytes.begin);
    
    handler->think(handler,state,state->read.bytes.begin);
}

static void cb_finished_write(tcp_event_connection_state * state)
{
    
}

static void cb_disconnect(tcp_event_connection_state * state)
{
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
