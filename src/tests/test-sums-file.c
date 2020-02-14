#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "hash_table_string.h"
#include "dictionary.h"
#include "sums_file.h"

int main(int argc, char * argv[])
{
    if( argc != 2 )
    {
	printf("Need a test file arg\n");
	exit(1);
    }

    const char * file_name = argv[1];

    FILE * file;

    assert(NULL != (file = fopen(file_name,"r")));

    sums_db db = { 0 };

    assert(0 == sums_load(&db,file));

    fclose(file);

    printf("Dumping sums:\n");
    sums_dump(stdout,&db);

    return 0;
}
