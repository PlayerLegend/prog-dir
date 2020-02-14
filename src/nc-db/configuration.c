#define FLAT_INCLUDES

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    *set_option_string(&options,CONFIG_PORT) = dupe_string("7793");
}

int init_config(int argc, char * argv[])
{
    options_init(&options);
    
    set_defaults();
    
    int opt;

    while( -1 != (opt = getopt(argc,argv,"f:p:")) )
	switch(opt)
	{
	case 'p':
	    printf("Set port %s\n",optarg);
	    *set_option_string(&options,CONFIG_PORT) = dupe_string(optarg);
	    break;
	default:
	    break;
	}

    return 0;
}

const char * config_string(char * name)
{
    return get_option_string(&options,name);
}
