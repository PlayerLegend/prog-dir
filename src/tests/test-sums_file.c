#define FLAT_INCLUDES
#include <stdio.h>
#include <string.h>
#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#include "dictionary.h"
#include "sums_file.h"
#include "line_parse.h"
#include "for_range.h"

int main(int argc, char * argv[])
{
    //printf("Starting test of %s\n",argv[0]);
    
    if(argc < 2)
	return 1;

    const char * file_name = argv[1];

    FILE * commands_file = stdin;    
    if(argc >= 3)
    {
	commands_file = fopen(argv[2],"r");
	if(!commands_file)
	{
	    perror(argv[2]);
	    fflush(stdout);
	    return 1;
	}
    }

    FILE * file = fopen(file_name,"r");

    if(!file)
	return 1;

    sums_db db;
    sums_db_init(&db);

    sums_file_load(&db,file);

    //printf("%zd buckets in dictionary\n",db.key_table.end - db.key_table.begin);
    //printf("%zd relations in dictionary\n",db.key_table.key.end - db.key_table.key.begin);
    //printf("%zd indexes allocated to outputs\n",db.value_map.alloc - db.value_map.begin);
    
    char * text = NULL;
    size_t len = 0;

    delimited_string delimited = {0};
    
    while( getline(&text,&len,commands_file) != -1 )
    {
	clean_trailing(text,'\n');
	delimit(&delimited,text,' ');
	print_delimited(&delimited);

	if( strcmp(delimited.begin[0],"sums") == 0 )
	{
	    if(delimited.end - delimited.begin != 2)
	    {
		printf("Incorrect number of arguments, want 2 and got %zd\n", delimited.end - delimited.begin);
		continue;
	    }
	    
	    sums_relation * relation = dictionary_access_key(&db,delimited.begin[1]);
	    printf("Listing %zd sums for '%s':\n",table_keycount(&relation->sums),delimited.begin[1]);

	    for_range(i,relation->sums)
	    {
		if(i->state != TABLE_BUCKET_FILLED)
		    continue;

		size_t key = (size_t)relation->sums.key.begin[i->key_index];
		
		printf("\t%s\n",dictionary_keyof_index(&db,key));
	    }
	    printf("Done\n");
	}
	else if( strcmp(delimited.begin[0],"names") == 0 )
	{
	    if(delimited.end - delimited.begin != 2)
	    {
		printf("Incorrect number of arguments\n");
		continue;
	    }
	    
	    sums_relation * relation = dictionary_access_key(&db,delimited.begin[1]);
	    printf("Listing %zd names for '%s':\n",table_keycount(&relation->names),delimited.begin[1]);

	    for_range(i,relation->names)
	    {
		if(i->state != TABLE_BUCKET_FILLED)
		    continue;

		size_t key = (size_t)relation->names.key.begin[i->key_index];
		
		printf("\t%s\n",dictionary_keyof_index(&db,key));
	    }
	    printf("Done\n");
	}
	else
	{
	    printf("unknown command '%s'\n",delimited.begin[0]);
	}
    }
    
    fclose(file);
}
