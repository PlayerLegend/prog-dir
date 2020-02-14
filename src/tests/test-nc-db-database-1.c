
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "hash_table_string.h"
#include "dictionary.h"
#include "nc-db/database.h"

int main(int argc, char * argv[])
{
    if( argc != 3 )
    {
	printf("Need a test file and nonexistent file arg\n");
	exit(1);
    }

    const char * file_name = argv[1];
    const char * no_file = argv[2];

    db_handle handle;

    char add_line[] = "sum-part  info part";
    
    assert( 0 == db_load(&handle,file_name) );
    assert( 0 == db_add(handle,add_line) );
    assert( 0 == db_dump(handle) );

    db_handle handle2;

    assert( 0 == db_load(&handle2,no_file) );
    assert(handle2 != handle);
    char add_line2[] = "asdf  987234 0918234";
    assert( 0 == db_add(handle2,add_line2) );
    assert( 0 == db_dump(handle2) );

    struct stat s;

    return 0;
}
