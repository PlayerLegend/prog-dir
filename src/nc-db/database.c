#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "range.h"
#include "nc-db/index_string.h"
#include "nc-db/database.h"
#include "print.h"

typedef struct {
    ino_t inode;
    dev_t device;
}
    file_uid;

typedef struct {
    bool is_loaded;
    bool is_single;
    union {
	db_multiple multiple;
	db_single single;
    };
}
    loaded_db_entry;

dictionary(loaded_db_entry) loaded_db;

static size_t table_digest_file_uid(const void * key)
{
    const file_uid * uid = key;
    return uid->inode * uid->device;
}

static int table_equals_file_uid(const void * a, const void * b)
{
    const file_uid *uid_a = a, *uid_b = b;

    return uid_a->inode == uid_b->inode && uid_a->device == uid_b->device;
}

static int table_copy_file_uid(void ** dst, const void * src)
{
    *dst = malloc(sizeof(file_uid));

    if(!*dst)
    {
	perror("malloc");
	return -1;
    }
    
    memcpy(*dst,src,sizeof(file_uid));
    return 0;
}

#define TABLE_CONFIG_FILE_UID			\
    (table_config){				\
	.gen_digest = table_digest_file_uid,	\
	    .copy = table_copy_file_uid,		\
	    .equals = table_equals_file_uid,	\
	    .free = free	\
	    }

static int get_file_uid_size(file_uid * uid, size_t * size, const char * path)
{
    struct stat s;
    if( -1 == stat(path,&s) )
    {
	perror(path);
	return -1;
    }

    uid->inode = s.st_ino;
    uid->device = s.st_dev;
    *size = s.st_size;

    return 0;
}

void init_db()
{
    dictionary_config(&loaded_db) = TABLE_CONFIG_FILE_UID;
}

#define get_value(dbp,value)			\
    dictionary_access_key(&(dbp)->forward,(void*)(value))

#define get_key(dbp,key)				\
    dictionary_access_key(&(dbp)->reverse,(void*)(key))


#define index_value(dbp,value)			\
    dictionary_indexof_key(&(dbp)->forward,(void*)(value))

#define index_key(dbp,key)				\
    dictionary_indexof_key(&(dbp)->reverse,(void*)(key))


void db_add_single(db_single * db, size_t key, size_t value)
{
    size_t * set;

    set = get_value(db,key);
    size_t have = *set;

    if( have == 0 )
	goto INSERT;
    
    if( have == value )
	return;

    *get_key(db,have) = 0;
    set = get_value(db,key);

INSERT:
    *get_key(db,value) = key;
    *set = value;
}

void db_add_multiple(db_multiple * db, size_t key, size_t value)
{
    size_t * set = get_key(db,value);
    size_t have = *set;

    if( have == 0 )
	goto INSERT;

    if( have == key )
	return;

    index_array * delete = get_value(db,have);
    for_range(i,*delete)
    {
	if(*i == value)
	{
	    array_flip_del(delete,i);
	    break;
	}
    }

    set = get_key(db,value);
    
INSERT:
    
    *set = key;
    index_array * add = get_value(db,key);
    *array_push(add) = value;
}

static int init_path(const char * file_name, const char * line)
{
    FILE * init_file;
    if(NULL == (init_file = fopen(file_name,"w")))
    {
	perror(file_name);
	return -1;
    }
    fprintf(init_file,"%s\n",line);
    fclose(init_file);
    return 0;
}

static int find_db(loaded_db_entry ** entry, const char * file_name, bool is_single, const char * header)
{
    file_uid uid;
    size_t file_size;
    if( -1 == get_file_uid_size(&uid,&file_size,file_name) &&
	-1 == init_path(file_name,header) &&
	-1 == get_file_uid_size(&uid,&file_size,file_name) )
	return -1;

    if( 0 == file_size && -1 == init_path(file_name,header) )
	return -1;
    
    *entry = dictionary_access_key(&loaded_db,&uid);

    if( (*entry)->is_loaded )
    {
	if( (*entry)->is_single != is_single )
	{
	    print_error("Database is not of type '%s', '%s'",header,file_name);
	    return -1;
	}
    }
    else
    {
	(*entry)->is_single = is_single;
    }

    return 0;
}

static FILE * open_database(const char * file_name, const char * header)
{
    FILE * file;
    
    struct { char * text; size_t len; } line = {0};
    
    if( NULL == (file = fopen(file_name,"r")) )
    {
	perror(file_name);
	return NULL;
    }

    if( -1 == getline(&line.text,&line.len,file) )
    {
	perror(file_name);
	print_error("Failed to read database %s\n",file_name);
	free(line.text);
	fclose(file);
	return NULL;
    }

    if( 0 != strcmp(line.text,header) )
    {
	print_error("Incorrect database type, needed '%s', got '%s'\n",header,line.text);
	free(line.text);
	fclose(file);
	return NULL;
    }
    
    free(line.text);

    return file;
}

int db_load_multiple(db_multiple ** db, const char * file_name)
{
    loaded_db_entry * entry;

    if( -1 == find_db(&entry,file_name,false,"multiple") )
	return -1;

    if(entry->is_loaded)
    {
	*db = &entry->multiple;
	return 0;
    }

    FILE * file;
    if( NULL == (file = open_database(file_name,"multiple")) )
    {
	perror(file_name);
	return -1;
    }

    struct { char * text; size_t len; } line = {0};
    size_t key;
    table * table = get_string_table();

    while( -1 != getline(&line.text,&line.len,file) )
    {
	if( '\0' == *line.text )
	{
	    key = 0;
	}
	else if(0 == key)
	{
	    key = table_include(table,line.text);
	}
	else
	{
	    db_add_multiple(&entry->multiple,key,table_include(table,line.text));
	}
    }

    if(ferror(file))
    {
	perror(file_name);
	goto ERROR;
    }

    fclose(file);
    free(line.text);
    
    return 0;

ERROR:
    if(file)
	fclose(file);
    free(line.text);
    return -1;
}

int db_load_single(db_single ** db, const char * file_name)
{
    loaded_db_entry * entry;

    if( -1 == find_db(&entry,file_name,false,"single") )
	return -1;

    if(entry->is_loaded)
    {
	*db = &entry->single;
	return 0;
    }

    FILE * file;
    if( NULL == (file = open_database(file_name,"single")) )
    {
	perror(file_name);
	return -1;
    }

    struct { char * text; size_t len; } line = {0};
    size_t key = 0;
    bool full = false;
    table * table = get_string_table();

    int lno = 0;

    while( -1 != getline(&line.text,&line.len,file) )
    {
	lno++;
	
	if( *line.text == '\0' )
	{
	    key = 0;
	    full = false;
	}
	else if(key == 0)
	{
	    key = table_include(table,line.text);
	}
	else if(!full)
	{
	    db_add_single(&entry->single, key, table_include(table,line.text));
	}
	else
	{
	    print_error("Excess element in single database entry on line %d, offending element is '%s'",lno,line.text);
	    goto ERROR;
	}
    }

    if(ferror(file))
    {
	perror(file_name);
	goto ERROR;
    }

    fclose(file);
    free(line.text);
    
    return 0;

ERROR:
    if(file)
	fclose(file);
    free(line.text);
    return -1;
}
