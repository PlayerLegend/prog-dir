#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "hash_table_string.h"
#include "dictionary.h"
#include "sums_file.h"
#include "range.h"
#include "print_array.h"
#include "print.h"
#include "nc-db/database.h"

typedef struct {
    pthread_mutex_t lock;
    sums_db db;
    const char * path;
}
    loaded_file;

static struct {
    dictionary(loaded_file) files;
    pthread_mutex_t lock;
}
    static_vars = { .files.keys.config = TABLE_CONFIG_STRING, .lock = PTHREAD_MUTEX_INITIALIZER };

static loaded_file * get_file(db_handle handle)
{
    loaded_file * ret;
    pthread_mutex_lock(&static_vars.lock);
    ret = dictionary_access_index(&static_vars.files,handle);
    pthread_mutex_unlock(&static_vars.lock);
    return ret;
}

static void destroy_loaded_file(loaded_file * file)
{
    pthread_mutex_destroy(&file->lock);
    table_clear(&file->db.keys);
}

int db_load(db_handle * handle, const char * path)
{
    pthread_mutex_lock(&static_vars.lock);
    *handle = dictionary_indexof_key(&static_vars.files,path);
    loaded_file * loaded_file = dictionary_access_index(&static_vars.files,*handle);
    assert(loaded_file != NULL);
    dictionary_config(&loaded_file->db) = TABLE_CONFIG_STRING;
    pthread_mutex_init(&loaded_file->lock,NULL);
    loaded_file->path = dictionary_keyof_index(&static_vars.files,*handle);

    FILE * file;

    if( NULL == (file = fopen(path,"r")) )
    {
	if( errno == ENOENT )
	{
	    pthread_mutex_unlock(&static_vars.lock);
	    return 0;
	}
	
	perror(path);
	goto ERROR;
    }

    static struct { char * text; size_t len; } line;

    while( -1 != getline(&line.text,&line.len,file) )
    {
	if( -1 == sums_add(&loaded_file->db,line.text) )
	{
	    fclose(file);
	    goto ERROR;
	}
    }

    if(ferror(file))
    {
	perror(path);
	fclose(file);
	goto ERROR;
    }

    fclose(file);
    
    pthread_mutex_unlock(&static_vars.lock);
    return 0;

ERROR:
    print_error("Loading failed for %s\n",path);
    destroy_loaded_file(loaded_file);
    dictionary_delete(&static_vars.files,(char*)path);
    pthread_mutex_unlock(&static_vars.lock);
    return -1;
}

int db_add(db_handle handle, char * line)
{
    int ret;
    loaded_file * file = get_file(handle);

    pthread_mutex_lock(&file->lock);

    ret = sums_add(&file->db,line);
    
    pthread_mutex_unlock(&file->lock);

    return ret;
}

int db_delete(db_handle handle, char * line)
{
    int ret;
    loaded_file * file = get_file(handle);

    pthread_mutex_lock(&file->lock);

    ret = sums_delete(&file->db,line);
    
    pthread_mutex_unlock(&file->lock);

    return ret;
}
    
void db_get_values(char_array * output, db_handle handle, const char * key)
{
    loaded_file * file = get_file(handle);

    sums_entry * entry;
    
    pthread_mutex_lock(&file->lock);

    entry = dictionary_access_key(&file->db,key);
    for_range(value,entry->value)
	print_array_append(output,"%s  %s\n",key,*value);
    
    pthread_mutex_unlock(&file->lock);
}

void db_get_keys(char_array * output, db_handle handle, const char * value)
{
    loaded_file * file = get_file(handle);

    sums_entry * entry;
    
    pthread_mutex_lock(&file->lock);

    entry = dictionary_access_key(&file->db,value);
    for_range(key,entry->key)
	print_array_append(output,"%s  %s\n",*key,value);
    
    pthread_mutex_unlock(&file->lock);
}

int db_dump(db_handle handle)
{
    loaded_file * loaded_file;
    pthread_mutex_lock(&static_vars.lock);
    loaded_file = dictionary_access_index(&static_vars.files,handle);
    pthread_mutex_lock(&loaded_file->lock);
    pthread_mutex_unlock(&static_vars.lock);
    
    FILE * file;

    if( NULL == (file = fopen(loaded_file->path,"w")) )
    {
	perror(loaded_file->path);
	goto ERROR;
    }

    if( -1 == sums_dump(file,&loaded_file->db) )
    {
	fclose(file);
	goto ERROR;
    }
    
    fclose(file);

    printf("wrote %s\n",loaded_file->path);
    
    pthread_mutex_unlock(&loaded_file->lock);
    return 0;

ERROR:
    printf("Failed to dump %s\n",loaded_file->path);
    pthread_mutex_unlock(&loaded_file->lock);

    return -1;
}
