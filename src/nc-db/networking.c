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
#include "nc-db/database.h"
#include "nc-db/configuration.h"
#include "nc-db/index_string.h"

typedef struct {
    enum { NO_TYPE,
	   SET_SINGLE,
	   SET_MULTIPLE,
	   FWD_SINGLE,
	   FWD_MULTIPLE,
	   REV_SINGLE,
	   REV_MULTIPLE, } state;
    union {
	db_single * single;
	db_multiple * multiple;
    };
    size_t key;
    bool full;
}
    handler;

static void terminate(char_array * array)
{
    for_range(byte,*array)
    {
	if(!*byte)
	    return;
	
	if(*byte == '\n')
	{
	    *byte = '\0';
	    return;
	}
    }

    *array_push(array) = '\0';
}

static void cb_connect(tcp_event_connection_state * state)
{
    *array_push(&state->read.term_bytes) = '\n';
    state->custom.client = malloc(sizeof(handler));
    memset(state->custom.client,0,sizeof(handler));
}

static int connection_init(handler * handler, char * message)
{
    
    char *subject, *predicate;
    subject = message;
    predicate = strchr(subject,' ');
    if( NULL == predicate )
    {
	print_error("Failed to parse request '%s'",message);
	return -1;
    }

    *predicate = '\0';
    predicate++;
	
    if( 0 == strcmp(subject,"set-multiple") )
    {
	if( -1 == db_load_multiple(&handler->multiple,predicate) )
	{
	    print_error("Failed to load multiple database for '%s %s'",subject,predicate);
	    return -1;
	}
	    
	handler->state = SET_MULTIPLE;
    }
    else if( 0 == strcmp(subject,"set-single") )
    {
	if( -1 == db_load_single(&handler->single,predicate) )
	{
	    print_error("Failed to load single database for '%s %s'",subject,predicate);
	    return -1;
	}
    }
    else if( 0 == strcmp(subject,"get-multiple") )
    {
	if( -1 == db_load_multiple(&handler->multiple,predicate) )
	{
	    print_error("Failed to load multiple database for '%s %s'",subject,predicate);
	    return -1;
	}

	handler->state = FWD_MULTIPLE;
    }
    else if( 0 == strcmp(subject,"get-single") )
    {
	
	if( -1 == db_load_single(&handler->single,predicate) )
	{
	    print_error("Failed to load single database for '%s %s'",subject,predicate);
	    return -1;
	}
	
	handler->state = FWD_SINGLE;
    }
    else
    {
	print_error("Unknown opener '%s %s'\n",subject,predicate);
	return -1;
    }

    return 0;
}

static int set_single(table * table, handler * handler, char * message)
{
    size_t message_index;
    if( '\0' == *message )
    {
	handler->key = 0;
	handler->full = false;
	return 0;
    }

    if(handler->full)
    {
	print_error("Excess element in pair, '%s'",message);
	return -1;
    }

    message_index = table_include(table,message);
	
    if(handler->key == 0)
    {
	handler->key = message_index;
    }
    else if(!handler->full)
    {
	db_add_single(handler->single,handler->key,message_index);
	handler->full = true;
    }

    return 0;
}

static int set_multiple(table * table, handler * handler, char * message)
{
    if( '\0' == *message )
    {
	handler->key = 0;
	return 0;
    }

    size_t message_index = table_include(table,message);

    if( 0 == handler->key )
    {
	handler->key = message_index;
	return 0;
    }

    db_add_multiple(handler->multiple,handler->key,message_index);

    return 0;
}

static void cb_finished_read(tcp_event_connection_state * state)
{
    handler * handler = state->custom.client;
    table * table = state->custom.server;
    
    terminate(&state->read.bytes);
    char *message = state->read.bytes.begin;

    size_t key;
    size_t value;
    const char * name;
    index_array * array;
    
    switch(handler->state)
    {
    case NO_TYPE:
	if( -1 == connection_init(handler,message) )
	    goto ERROR;
	return;

    case SET_SINGLE:
	if( -1 == set_single(table,handler,message) )
	    goto ERROR;
	return;

    case SET_MULTIPLE:
	if( -1 == set_multiple(table,handler,message) )
	    goto ERROR;
	return;

    case FWD_SINGLE:
	key = table_include(table,message);
	value = *dictionary_access_key(&handler->single->forward,(void*)key);
	name = table_keyof_index(table,value);
	print_array_append(&state->write.bytes,"%s  %s\n",message,name);
	state->write.active = true;
	return;
	
    case FWD_MULTIPLE:
	key = table_include(table,message);
	array = dictionary_access_key(&handler->multiple->forward,(void*)key);
	for_range(i,*array)
	{
	    name = table_keyof_index(table,*i);
	    print_array_append(&state->write.bytes,"%s  %s\n",message,name);
	}
	state->write.active = true;
	return;

    case REV_SINGLE:
	value = table_include(table,message);
	key = *dictionary_access_key(&handler->single->reverse,(void*)value);
	name = table_keyof_index(table,key);
	print_array_append(&state->write.bytes,"%s  %s\n",name,message);
	state->write.active = true;
	return;
	
    case REV_MULTIPLE:
	value = table_include(table,message);
	key = *dictionary_access_key(&handler->multiple->reverse,(void*)value);
	name = table_keyof_index(table,key);
	print_array_append(&state->write.bytes,"%s  %s\n",name,message);
	state->write.active = true;
	return;
	
    default:
	print_error("Unknown value in switch");
	goto ERROR;
    }

    printf("Missed switch");
    
ERROR:
    state->disconnect = true;
    return;
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
	    .custom = get_string_table(),
	    .connect = cb_connect,
	    .finished_read = cb_finished_read,
	    .finished_write = cb_finished_write,
	    .disconnect = cb_disconnect,
	};

    const char * port = config_string(CONFIG_PORT);
    printf("Listening on %s\n",port);
    
    return tcp_event_listen(port,&event_config);
}
