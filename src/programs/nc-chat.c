#include <stdio.h>
#include <stdbool.h>

#include "stack.h"
#include "array.h"
#include "tcp_event.h"
#include "print.h"
#include "print_array.h"
#include "index_map.h"
#include "hash_table.h"
#include "hash_table_string.h"
#include "dictionary.h"
#include "for_range.h"

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdarg.h>

const char * server_username = "server";

void terminate(char_array * array)
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

typedef struct {
    array(char*) pending_messages;
    char * password;
    tcp_event_connection_state * state;
    unsigned int login_count;
}
    user_info;

typedef struct {
    user_info * user;
    enum {
	CONNECTION_ERROR_STATE,
	CONNECTION_GET_KNOWN_PASSWORD,
	CONNECTION_GET_NEW_PASSWORD,
	CONNECTION_CONFIRM_NEW_PASSWORD,
	CONNECTION_LOGGED_IN,
    } state;
}
    connection_custom;

dictionary(user_info) user_db;
pthread_mutex_t user_db_lock = PTHREAD_MUTEX_INITIALIZER;

char * dupe_string(const char * string)
{
    return strcpy(malloc(strlen(string) + 1),string);
}

void message_everyone(const char * fmt,...)
{
    char_array send = { 0 };
    va_list va;
    va_start(va,fmt);
    vprintf(fmt,va);
    //print_append_array(&send,fmt,va);
    va_end(va);
    
//    printf("BUFFER: '%.*s'",(int)count_range(send),send.begin);
    
    pthread_mutex_lock(&user_db_lock);

    user_info * user_info;

    for_range(bucket,user_db.keys)
    {
	if(bucket->state != TABLE_BUCKET_FILLED)
	{
	    continue;
	}

	user_info = dictionary_access_index(&user_db,bucket->key_index);

	if(user_info->password && user_info->state)
	{
	    *array_push(&user_info->pending_messages) = dupe_string(send.begin);
	    tcp_event_wake_write(user_info->state);
	}
    }
    
    pthread_mutex_unlock(&user_db_lock);
    free(send.begin);
}

int message_client(tcp_event_connection_state * client, const char * fmt, ...)
{
    int ret;
    va_list va;
    va_start(va,fmt);
    ret = print_array_append(&client->write.bytes,fmt,va);
    va_end(va);
    client->write.active = true;
    return ret;
}

user_info * retrieve_info(void * custom)
{
    size_t index = (size_t)custom - 1;
    return dictionary_access_index(&user_db,index);
}

bool login_user(tcp_event_connection_state * state)
{
    connection_custom * login = state->custom.client;
    const char * message = state->read.bytes.begin;
    
    if(!login) // no name yet
    {
	printf("no login\n");
	login = malloc(sizeof(*login));
	*login = (connection_custom){};
	state->custom.client = login;
	message_client(state,"*knock knock*\nWho goes there?\n> ");
	
	return false;
    }
    
    if(!login->user)
    {
	printf("no user\n");
	if( 0 == strcmp(state->read.bytes.begin,server_username) )
	{
	    message_client(state,"Haha, a joker! Try again.\n> ");
	    return false;
	}
	
	login->user = dictionary_access_key(&user_db,message);
	
	if(!login->user->password)
	{
	    message_client(state,"%s eh? Alright, what's the secret password?\n> ",message);
	    login->state = CONNECTION_GET_NEW_PASSWORD;
	}
	else
	{
	    message_client(state,"Ah, %s. What's the secret password?\n> ",state->read.bytes.begin);
	    login->state = CONNECTION_GET_KNOWN_PASSWORD;
	}

	return false;
    }

    printf("switch\n");
    
    user_info * user = login->user;
    
    switch(login->state)
    {
    case CONNECTION_GET_NEW_PASSWORD:
	user->password = dupe_string(message);
	message_client(state,"Ahem, say again?\n> ");
	login->state = CONNECTION_CONFIRM_NEW_PASSWORD;
	break;

    case CONNECTION_CONFIRM_NEW_PASSWORD:
	if( 0 == strcmp(message,user->password) )
	{
	    message_client(state,"Welcome...\n");
	    login->state = CONNECTION_LOGGED_IN;
	}
	else
	{
	    message_client(state,"I don't think I heard you right, speak up!\n> ");
	    free(user->password);
	    user->password = NULL;
	    login->state = CONNECTION_GET_NEW_PASSWORD;	    
	}
	break;
    case CONNECTION_GET_KNOWN_PASSWORD:
	if( 0 == strcmp(message,user->password) )
	{
	    message_client(state,"Alright, you're in.\n");
	    login->state = CONNECTION_LOGGED_IN;	    
	}
	else
	{
	    message_client(state,"Nope! Get lost.\n");
	    state->read.active = false;
	}
	break;
	
    case CONNECTION_LOGGED_IN:
	return true;

    default:
	message_client(state,"Oops, I think I dropped something.\n");
	state->read.active = false;
	break;
    }
    
    return false;  
}

bool cb_connect(tcp_event_connection_state * client)
{
    login_user(client);
    return true;
}

bool cb_finished_read(tcp_event_connection_state * state)
{
    terminate(&state->read.bytes);
    if(!login_user(state))
	return true;
    
    connection_custom * login = state->custom.client;
    user_info * user = login->user;
    const char * username = dictionary_keyof_value(&user_db,user);

    message_everyone("SAY: %s: %s\n",username,state->read.bytes.begin);
    
    return true;
}

bool cb_finished_write(tcp_event_connection_state * state)
{
    connection_custom * login = state->custom.client;

    if(!login || login->state != CONNECTION_LOGGED_IN)
	return true;
    
    user_info * user = login->user;

    if(!is_range_empty(user->pending_messages))
    {
	for_range(message,user->pending_messages)
	{
	    print_array_append(&state->write.bytes,*message);
	    free(*message);
	}

	array_rewrite(&user->pending_messages);

	state->write.active = true;
    }
    
    return true;
}

void cb_disconnect(tcp_event_connection_state * state)
{
    connection_custom * login = state->custom.client;

    if(!login || login->state != CONNECTION_LOGGED_IN)
	return;

    user_info * user = login->user;

    user->login_count--;
    if(user->login_count == 0)
    {
	const char * username = dictionary_keyof_value(&user_db,user);

	message_everyone("%s: %s disconnected\n",server_username,username);
    }
}

int main(int argc, char * argv[])
{
    printf("%sv0.1a\n",argv[0]);
    
    if(argc < 2 || argc > 3)
    {
	print_error("usage: %s [port] [optional:motd]\n",argv[0]);
	exit(1);
    }

    dictionary_config(&user_db) = TABLE_CONFIG_STRING;

    tcp_event_config config =
    {
	.connect = cb_connect,
	.finished_read = cb_finished_read,
	.finished_write = cb_finished_write,
	.disconnect = cb_disconnect,
    };

    tcp_event_listen(argv[1],&config);
}
