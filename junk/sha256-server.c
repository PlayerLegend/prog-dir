#include "network.h"
#include "sha256.h"
#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "print.h"
#include "for_range.h"

typedef struct client {
    struct {
	char * text;
	size_t len;
    }
	file_name;
    struct client * watch;
    sha256_armor armor;
    sha256_job job;
    FILE * talk;
}
    client;

struct {
    array(client) todo;
    sem_t semaphore;
    pthread_mutex_t lock;
}
    clients;

#define FILE_BUFFER_SIZE (1 << 20)

#define lock_clients()				\
    pthread_mutex_lock(&clients.lock)

#define unlock_clients()				\
    pthread_mutex_unlock(&clients.lock)

dictionary(char*) sums_dictionary;
pthread_mutex_t sums_dictionary_lock = PTHREAD_MUTEX_INITIALIZER;

char * lookup_sum(const char * filename)
{
    char * ret;
    pthread_mutex_lock(&sums_dictionary_lock);
    ret = *dictionary_access_key(&sums_dictionary,filename);
    pthread_mutex_unlock(&sums_dictionary_lock);
    return ret;
}

void set_sum(const char * filename, const char * sum)
{
    char ** get;
    char * set = strcpy(malloc(strlen(sum) + 1),sum);
    pthread_mutex_lock(&sums_dictionary_lock);
    get = dictionary_access_key(&sums_dictionary,filename);
    if(*get)
	free(*get);
    *get = set;
    pthread_mutex_unlock(&sums_dictionary_lock);
}

int client_halt(client * cli)
{
    free(cli->file_name.text);
    fclose(cli->talk);
    sha256_halt_partial(&cli->job);
}

int client_frame(client * cli)
{
    if(ferror(cli->talk))
    {
	client_halt(cli);
	return -1;
    }
    
    if(feof(cli->talk))
    {
	client_halt(cli);
	return 1;
    }

    if(cli->watch)
    {
	if( 0 == strcmp(cli->watch->file_name.text,cli->file_name.text) )
	{
	    return 0;
	}

	char * sum;

	if( NULL != (sum = lookup_sum(cli->file_name.text)) )
	{
	    fprintf(cli->talk,"%s\n",sum);
	    return 0;
	}

	print_error("watch missed");
    }

    if(!cli->job.file)
    {
	if( -1 == getline(&cli->file_name.text, &cli->file_name.len, cli->talk) )
	{
	    return -1;
	}

	for_range(other,clients.todo)
	{
	    if( other->file_name.text && 0 == strcmp(other->file_name.text,cli->file_name.text) )
	    {
		cli->watch = other;
		return 0;
	    }
	}

	if( NULL == (cli->job.file = fopen(cli->file_name.text,"r")) )
	{
	    fprintf(cli->talk,"error\nno file %s",cli->file_name.text);
	    return -1;
	}
    }

    if( 0 != sha256_partial(&cli->job) )
    {
	fclose(cli->job.file);
	cli->job.file = NULL;

	if(cli->job.success)
	{
	    sha256_makearmor(cli->armor,cli->job.sum);
	    fprintf(cli->talk,"%s\n",cli->armor);
	    return 1;
	}
	else
	{
	    fprintf(cli->talk,"error\nfailed to generate sum");
	    client_halt(cli);
	    return -1;
	}
    }

    return 0;
}

void client_start(FILE * talk)
{
    *array_push(&clients.todo) = (client)
    {
	.talk = talk,
	.job = { .buffer = malloc(FILE_BUFFER_SIZE),
		 .buffer_size = FILE_BUFFER_SIZE, },
    };
}

