#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "hash_table_string.h"
#include "dictionary.h"
#include "range.h"
#include "print.h"
#include "delimit.h"
#include "nc-db/database.h"

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

static struct {
    dictionary(loaded_db_entry) loaded_db;
    table string_table;
}
    static_var;

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

static void terminate(char * text)
{
    while( *text && *text != '\n' )
	text++;
    *text = '\0';
}

static void clean_db()
{
    
}

int init_db()
{
    memset(&static_var,0,sizeof(static_var));
    dictionary_config(&static_var.loaded_db) = TABLE_CONFIG_FILE_UID;
    static_var.string_table.config = TABLE_CONFIG_STRING;
    atexit(clean_db);
    if(0 != table_include(&static_var.string_table,""))
    {
	return -1;
    }

    return 0;
}

#define get_value(dbp,value)			\
    dictionary_access_key(&(dbp)->forward,(void*)(value))

#define get_key(dbp,key)				\
    dictionary_access_key(&(dbp)->reverse,(void*)(key))


#define index_value(dbp,value)			\
    dictionary_indexof_key(&(dbp)->forward,(void*)(value))

#define index_key(dbp,key)				\
    dictionary_indexof_key(&(dbp)->reverse,(void*)(key))


void db_add_single(db_single * db, key_value kv)
{
    size_t * set;

    set = get_value(db,kv.key);
    size_t have = *set;

    if( have == 0 )
	goto INSERT;
    
    if( have == kv.value )
	return;

    *get_key(db,have) = 0;
    set = get_value(db,kv.key);

INSERT:
    *get_key(db,kv.value) = kv.key;
    *set = kv.value;
}

void db_add_multiple(db_multiple * db, key_value kv)
{
    size_t * set = get_key(db,kv.value);
    size_t have = *set;

    if( have == 0 )
	goto INSERT;

    if( have == kv.key )
	return;

    index_array * delete = get_value(db,have);
    for_range(i,*delete)
    {
	if(*i == kv.value)
	{
	    array_flip_del(delete,i);
	    break;
	}
    }

    set = get_key(db,kv.value);
    
INSERT:
    
    *set = kv.key;
    index_array * add = get_value(db,kv.key);
    *array_push(add) = kv.value;
}

void db_delete_multiple(db_multiple * db, key_value kv)
{
    size_t * rev = get_key(db,kv.value);
    if(*rev != kv.key)
	return;
    *rev = 0;
    index_array * delete = get_value(db,kv.key);
    for_range(i,*delete)
    {
	if(*i == kv.value)
	{
	    array_flip_del(delete,i);
	    return;
	}
    }
}

void db_delete_single(db_single * db, key_value kv)
{
    size_t * fwd = get_value(db,kv.key);
    if(*fwd != kv.value)
	return;
    *fwd = 0;
    *get_key(db,kv.value) = 0;
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

static int find_db(loaded_db_entry ** entry, const char * file_name, bool is_single, const char * header)
{
    file_uid uid;
    size_t file_size;
    
    if( -1 == get_file_uid_size(&uid,&file_size,file_name) )
    {
	if( -1 == init_path(file_name,header) || -1 == get_file_uid_size(&uid,&file_size,file_name) )
	    return -1;
    }
    else if( 0 == file_size && -1 == init_path(file_name,header) )
	return -1;
    
    *entry = dictionary_access_key(&static_var.loaded_db,&uid);

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

    terminate(line.text);

    if( 0 != strcmp(line.text,header) )
    {
	print_error("Incorrect database type at '%s', needed '%s', got '%s'\n",file_name,header,line.text);
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
	return -1;
    }

    key_value kv;
    
    struct { char * text; size_t len; } line = {0};

    while( -1 != getline(&line.text,&line.len,file) )
    {
	if( -1 == db_make_kv(&kv,line.text) )
	    goto ERROR;

	db_add_multiple(&entry->multiple,kv);
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
	return -1;
    }

    key_value kv;
    struct { char * text; size_t len; } line = {0};

    while( -1 != getline(&line.text,&line.len,file) )
    {
	if( -1 == db_make_kv(&kv,line.text) )
	    goto ERROR;

	db_add_single(&entry->single,kv);
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

int db_make_kv(key_value * kv, char * line)
{
    assert(NULL != line);
    assert(NULL != kv);
    
    clause_config clause_config =
	{
	    .separator_list = " ",
	    .separator_count = 2,
	};
    
    clause clause;

    terminate(line);
    
    if( -1 == delimit_clause(&clause,&clause_config,line) )
    {
	print_error("Failed to parse sum line '%s'\n",line);
	return -1;
    }

    *kv = (key_value)
    {
	.key = table_include(&static_var.string_table,clause.subject),
	.value = table_include(&static_var.string_table,clause.predicate),
    };

    return 0;
}

size_t db_index_string(const char * string)
{
    return table_include(&static_var.string_table,string);
}
