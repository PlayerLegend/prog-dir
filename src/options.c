#define FLAT_INCLUDES

#include <stdio.h>
#include <stdbool.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "delimit.h"
#include "options.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "hash_table_string.h"
#include "print.h"
#include "print_array.h"

#include <assert.h>

void options_init(option_db * db)
{
    *db = (option_db){ 0 };
    dictionary_config(db) = TABLE_CONFIG_STRING;
}

static void set_state(option_entry * entry, enum option_state new_state)
{
    if(entry->state != OPTION_NOT_SET && entry->state != new_state)
    {
	print_error("Altering the state of a configured option from %d -> %d",entry->state,new_state);
	if(entry->state == OPTION_STRING)
	    free(entry->strval);
    }

    entry->state = new_state;
}

char ** set_option_string(option_db * db, const char * name)
{
    printf("set %s\n",name);
    
    option_entry * entry = dictionary_access_key(db,name);
    set_state(entry,OPTION_STRING);
    return &entry->strval;
}

float * set_option_float(option_db * db, const char * name)
{
    option_entry * entry = dictionary_access_key(db,name);
    set_state(entry,OPTION_STRING);
    return &entry->floatval;
}

bool * set_option_bool(option_db * db, const char * name)
{
    option_entry * entry = dictionary_access_key(db,name);
    set_state(entry,OPTION_STRING);
    return &entry->boolval;
}

static char * dupe_string(const char * string)
{
    return strcpy(malloc(strlen(string) + 1),string);
}

static int eval_bool(bool * output, const char * string)
{
    char * true_strings[] = { "1", "y", "t", "true", "yes", NULL };
    char * false_strings[] = { "0", "n", "f", "false", "no", NULL };

    for( char ** check = true_strings; NULL != *check; check++ )
	if( 0 == strcasecmp(string,*check) )
	{
	    *output = true;
	    return 0;
	}
    
    for( char ** check = false_strings; NULL != *check; check++ )
	if( 0 == strcasecmp(string,*check) )
	{
	    *output = false;
	    return 0;
	}
    
    return -1;
}

int set_option_clause(option_db * db, const clause * clause)
{
    option_entry * entry = dictionary_access_key(db,clause->subject);
    bool set_bool;
    float set_float;
    char * endptr;
    
    switch(entry->state)
    {
    case OPTION_NOT_SET:
	print_error("Unrecognized option: %s\n",clause->subject);
	return -1;
	
    case OPTION_STRING:
	free(entry->strval);
	entry->strval = dupe_string(clause->predicate);
	return 0;

    case OPTION_FLOAT:
	set_float = strtof(clause->predicate,&endptr);
	if(!endptr || *endptr != '\0')
	{
	    print_error("%s requires a numeric value",clause->subject);
	    return -1;
	}
	else
	{
	    entry->floatval = set_float;
	    return 0;
	}

    case OPTION_BOOL:
	if( -1 == eval_bool(&set_bool,clause->predicate) )
	{
	    print_error("%s requires a boolean value",clause->subject);
	    return -1;
	}
	else
	{
	    entry->boolval = set_bool;
	    return 0;
	}

    default:
	print_error("%s is in an invalid state",clause->subject);
	return -1;
    }

    print_error("Switch statement is broken, clause: %s %s",clause->subject,clause->predicate);

    return -1;
}

static char find_c(char c, const char * list)
{
    if(!list)
	return '\0';
    
    char test;
    while( (test = *list++) )
	if(test == c)
	    break;

    return test;
}

bool is_comment(const char * text, const char * whitespace, char comment_char)
{
    while( find_c(*text,whitespace) )
	text++;

    return *text == comment_char;
}

int load_options_file(option_db * db, FILE * file)
{
    struct { char * text; size_t len; } line = {};
    clause clause;
    const char * whitespace = " \t";
    clause_config clause_config = { .separator_list = whitespace };
    
    while( -1 != getline(&line.text,&line.len,file) )
    {
	if(is_comment(line.text,whitespace,'#'))
	    continue;
	
	delimit_terminate(line.text,'\n');
	if( 0 != delimit_clause(&clause,&clause_config,line.text) ||
	    0 != set_option_clause(db,&clause) )
	{
	    free(line.text);
	    return -1;
	}
    }

    free(line.text);
    return 0;
}

static int get_rel_xdg(char_array * output, const char * path)
{
    char * rel = getenv("XDG_CONFIG_HOME");
    if(rel)
    {
	print_array_write(output,"%s/%s",rel,path);
	return 0;
    }

    rel = getenv("HOME");

    if(rel)
    {
	print_array_write(output,"%s/.config/%s",rel,path);
	return 0;
    }

    print_error("Unable to look up HOME or XDG_CONFIG_HOME environment variables");
    return -1;
}

static int get_rel_home(char_array * output, const char * path)
{
    char * rel = getenv("HOME");

    if(rel)
    {
	print_array_write(output,"%s/.%s",rel,path);
	return 0;
    }

    print_error("Unable to look up HOME environment variable");
    return -1;
}

static int get_rel_etc(char_array * output, const char * path)
{
    print_array_write(output,"/etc/%s",path);
}

static int find_file(char_array * output, const char * path)
{
    int (*get_rel[])(char_array * output, const char * path) =
    {
	get_rel_xdg,
	get_rel_home,
	get_rel_etc,
	NULL,
    };

    int (**i)(char_array * output, const char * path);

    for(i = get_rel; NULL != *i; i++)
    {
	if( -1 == (*i)(output,path) )
	    return -1;

	if( 0 == access(path,R_OK) )
	    return 0;
    }

    

    return -1;
}

char * find_config(const config_location * find)
{
    if(find->argc)
    {
	int start = optind;

	char val = find->opt_flag ? find->opt_flag : 1;

	char opt_string[] = { find->opt_flag, ':', '\0' };

	struct option opt_long[] =
	    { { .name = find->opt_long, .val = val, }, {} };

	int opt;
	
	while( -1 != (opt = getopt_long(find->argc,
				 find->argv,
				 opt_string,
				 opt_long,
					NULL)) )
	{
	    printf("asdf '%s' '%s'\n",find->opt_long, opt_string);
	    if(opt == val)
	    {
		return strcpy(malloc(strlen(optarg) + 1),optarg);
	    }
	}
	
	optind = start;
    }
    
    const char * path = find->path;
    
    while(*path == '/')
	path++;

    char_array output = {};
    if( -1 == find_file(&output,path) )
    {
	free(output.begin);
	return NULL;
    }

    return output.begin;
}

const char * get_option_string(option_db * db, const char * name)
{
    option_entry * entry = dictionary_access_key(db,name);

    printf("option: %d\n",entry->state);
    
    assert(entry->state == OPTION_STRING);

    return entry->strval;
}

float get_option_float(option_db * db, const char * name)
{
    option_entry * entry = dictionary_access_key(db,name);

    assert(entry->state == OPTION_FLOAT);

    return entry->floatval;
}

bool get_option_bool(option_db * db, const char * name)
{
    option_entry * entry = dictionary_access_key(db,name);

    assert(entry->state == OPTION_BOOL);

    return entry->boolval;
}
