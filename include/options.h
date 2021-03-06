#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <stdbool.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "delimit.h"

#endif

typedef struct {
    enum option_state { OPTION_NOT_SET, OPTION_STRING, OPTION_FLOAT, OPTION_BOOL } state;
    
    union {
	char * strval;
	float floatval;
	bool boolval;
    };
}
    option_entry;

typedef dictionary(option_entry) option_db;

typedef struct {
    const char * path;
    int argc;
    char ** argv;
    char opt_flag;
    const char * opt_long;
}
    config_location;

char * find_config(const config_location * find);

void options_init(option_db * db);

char ** set_option_string(option_db * db, const char * name);
float * set_option_float(option_db * db, const char * name);
bool * set_option_bool(option_db * db, const char * name);

const char * get_option_string(option_db * db, const char * name);
float get_option_float(option_db * db, const char * name);
bool get_option_bool(option_db * db, const char * name);

int set_option_clause(option_db * db, const clause * clause);
int load_options_file(option_db * db, FILE * file);
