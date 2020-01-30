#define FLAT_INCLUDES
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "tcp_event.h"
#include "print.h"
#include "sha256.h"
#include "range.h"
#include "print_array.h"
#include "delimit.h"
#include "options.h"

enum arg_options {
    ARG_PORT = 'p',
    ARG_DBFILE = 'd',
    ARG_RECORDFILE = 'r',
};

typedef struct {
    option_db options;
    char *service, *dbfile, *recordfile;
    struct {
	int count;
	char ** names;
    }
	files;
}
    program_config;

enum noise_tags {
    TAG_PATH,
    TAG_SHA256,
    LIMIT_TAG,
};

typedef array(size_t) index_array;

typedef struct {
    index_array tag[LIMIT_TAG]; /* map noise -> any */
    size_t noise; /* map any -> noise */
}
    db_entry;

typedef struct {
    pthread_mutex_t lock;
    dictionary(db_entry) dictionary;
}
    database;

#define OPT_ERROR_NOARG(c,type)					\
    {								\
	print_error("%c requires a [" type "] argument",c);	\
	exit(1);						\
    }

#define OPTION_DBFILE "database_file"
#define OPTION_RECORDFILE "record_file"
#define OPTION_PORT "port"

void read_program_args(program_config * config, int argc, char * argv[])
{
    int opt;
    char options[] = { ARG_PORT, ':',
		       ARG_DBFILE, ':',
		       ARG_RECORDFILE, ':',
		       '\0' };
    
    while( -1 != (opt = getopt(argc,argv,options)) )
	switch(opt)
	{
	case ARG_PORT:
	    if(!optarg)
		OPT_ERROR_NOARG(opt,"port");

	    *set_option_string(&config->options,OPTION_PORT) = optarg;
	    break;

	case ARG_DBFILE:
	    if(!optarg)
		OPT_ERROR_NOARG(opt,"database file");

	    *set_option_string(&config->options,OPTION_DBFILE) = optarg;
	    break;

	case ARG_RECORDFILE:
	    if(!optarg)
		OPT_ERROR_NOARG(opt,"record file");

	    *set_option_string(&config->options,OPTION_RECORDFILE) = optarg;
	    break;
	}
    
    config->files.count = argc - optind;
    config->files.names = argv + optind;
}

void database_delete(database * db, const char * noise, unsigned int tag_id, const char * tag_val)
{
    size_t val_index = dictionary_indexof_key(&db->dictionary,tag_val);
    index_array * tag_array = dictionary_access_key(&db->dictionary,noise)->tag + tag_id;

    for_range(i,*tag_array)
    {
	if(*i == val_index)
	{
	    array_flip_del(tag_array,i);
	    for_range_adjust(*tag_array);
	    for_range_redo(i);
	}
    }
}

void database_add(database * db, const char * noise, int tag_id, const char * tag_val)
{
    size_t val_index = dictionary_indexof_key(&db->dictionary,tag_val);
    index_array * tag_array = dictionary_access_key(&db->dictionary,noise)->tag + tag_id;

    for_range(i,*tag_array)
    {
	if(*i == val_index)
	    return;
    }

    *array_push(tag_array) = val_index;
}

char * dupe_string(char * string)
{
    return strcpy(malloc(strlen(string) + 1),string);
}

void database_export_table(table * output, database * db, char * tag_names[])
{
    db_entry * entry;
    index_array * array;
    size_t tag_index;
    char * tag_val;
    char * noise;
    char_array build = {};
    
    for_range(bucket,db->dictionary.keys)
    {
	if(bucket->state != TABLE_BUCKET_FILLED)
	    continue;
		
	noise = dictionary_keyof_index(&db->dictionary,bucket->key_index);
	entry = dictionary_access_index(&db->dictionary,bucket->key_index);
	
	for(tag_index = 0; tag_index < LIMIT_TAG; tag_index++)
	{
	    array = entry->tag + tag_index;
	    for_range(val_index,*array)
	    {
		tag_val = dictionary_keyof_index(&db->dictionary,*val_index);
		print_array_write(&build,"%s:%s  %s\n",
				   tag_names[tag_index],
				   noise,
				   tag_val);

		table_include(output,build.begin);
	    }
	}
    }

    free(build.begin);
}

int database_import_line(database * db, char * text, int tag_name_index[])
{
    char * tag_name = text;
    char * noise = strstr(tag_name,"  ");
    if(!noise)
	return -1;
    *noise = '\0';
    noise += 2;
    char * value = strstr(noise,"  ");
    if(!value)
    {
	noise[-2] = ' ';
	return -1;
    }
    *value = '\0';
    value += 2;

    int index = dictionary_indexof_key(&db->dictionary,tag_name);
    int tag;

    for(tag = 0; tag < LIMIT_TAG; tag++)
    {
	if(tag_name_index[tag] == index)
	    break;
    }

    if(tag == LIMIT_TAG)
	return -1;

    database_add(db,noise,tag,value);

    return 0;
}

int database_import_file(database * db, FILE * file, char * tag_names[])
{
    int tag_name_index[LIMIT_TAG];

    for(int i = 0; i < LIMIT_TAG; i++)
	tag_name_index[i] = dictionary_indexof_key(&db->dictionary,tag_names[i]);

    struct { size_t len; char * text; } line = {};
    
    while(-1 != getline(&line.text,&line.len,file))
    {
	if(-1 == database_import_line(db,line.text,tag_name_index))
	    print_error("Malformed database line: %s",line.text);
    }

    return 0;
}

char * get_config_path(char * path)
{
    char * ret = NULL;
    
    while(*path == '/')
	path++;

    char * rel = getenv("XDG_CONFIG_HOME");
    if(!rel)
    {
	rel = getenv("HOME");
	if(!rel)
	{
	    print_error("Environment variable HOME not set");
	    exit(1);
	}
	print_buffer_write(&ret,"%s/.config/sha256-daemon/%s",rel,path);
    }
    else
    {
	print_buffer_write(&ret,"%s/sha256-daemon/%s",rel,path);
    }

    return ret;
}

void set_defaults(option_db * options)
{
    *set_option_string(options,OPTION_PORT) = "7921";
    *set_option_string(options,OPTION_RECORDFILE) = ".sha256-daemon/records";
    *set_option_string(options,OPTION_DBFILE) = ".sha256-daemon/db";
}

void read_config_file(option_db * options, int argc, char * argv[])
{
    config_location config_location =
	{
	    .path = "sha256-daemon.conf",
	    .argc = argc,
	    .argv = argv,
	    .opt_flag = 'c',
	    .opt_long = "config",
	};

    char * config_path;

    if( NULL != (config_path = find_config(&config_location)) )
    {
	FILE * config = fopen(config_path,"r");

	if(!config)
	{
	    perror(config_path);
	    print_error("Error accessing config %s",config_location);
	    free(config_path);
	    exit(1);
	}

	if( -1 == load_options_file(options,config) )
	{
	    print_error("Error parsing config %s",config_location);
	    free(config_path);
	    fclose(config);
	    exit(1);
	}
	    
	free(config_path);
	fclose(config);
    }
}

int main(int argc, char * argv[])
{
    program_config config = {};
    options_init(&config.options);
    set_defaults(&config.options);    
    read_config_file(&config.options,argc,argv);
    read_program_args(&config,argc,argv);
    
    return 0;
}
