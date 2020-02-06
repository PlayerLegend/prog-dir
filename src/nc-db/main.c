#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "nc-db/index_string.h"
#include "nc-db/configuration.h"
#include "nc-db/database.h"
#include "nc-db/networking.h"

int main(int argc, char * argv[])
{
    init_string_index();
    init_config(argc,argv);
    init_db();

    printf("initialization done, starting network\n");

    if( -1 == network_listen() )
	return 1;

    return 0;
}