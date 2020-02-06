#define FLAT_INCLUDES

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "delimit.h"
#include "options.h"
#include "nc-db/configuration.h"

option_db options;

char * dupe_string(const char * string)
{
    return strcpy(malloc(strlen(string) + 1),string);
}

static void set_defaults()
{
    *set_option_string(&options,CONFIG_PORT) = dupe_string("7789");
}

int init_config(int argc, char * argv[])
{
    options_init(&options);
    
    set_defaults();
    
    config_location loc =
	{
	    .path = ".nc-db.conf",
	    .argc = argc,
	    .argv = argv,
	    .opt_flag = 'f',
	    .opt_long = "config",
	};

    char * path = find_config(&loc);

    if(!path)
    {
	return 0;
    }

    FILE * file;

    if( NULL != (file = fopen(path,"r")) )
    {
	if( -1 == load_options_file(&options,file) )
	{
	    free(path);
	    fclose(file);
	    return -1;
	}
    }

    free(path);
    fclose(file);
    return 0;
}

const char * config_string(char * name)
{
    return get_option_string(&options,name);
}
