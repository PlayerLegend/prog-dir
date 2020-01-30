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

#include "hash_table_string.h"
#include "print.h"

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

char ** set_option_str(option_db * db, const char * name)
{
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

int load_options_file(option_db * db, FILE * file)
{
    struct { char * text; size_t len; } line = {};
    clause clause;
    clause_config clause_config = { .separator_list = " \t" };
    
    while( -1 != getline(&line.text,&line.len,file) )
    {
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
