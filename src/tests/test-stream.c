#include "stream.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

void test_file_stream(const char * file_name)
{
    printf("Testing file streaming\n");
    
    FILE * file = fopen(file_name,"r");

    if(!file)
    {
	perror(file_name);
	exit(1);
    }

    stream _stream = stream_file_size(file,16);

    char c;

    while( (c = stream_c(_stream)) )
	printf("char: %c\n",c);

    for(int i = 0; i < 100; i++)
	assert(stream_c(_stream) == '\0');
}

void test_mem_stream()
{
    printf("Testing memory streaming\n");
    
    char * mem = "this is a string thats kinda long and gets the job done";

    stream _stream = stream_mem(mem,strlen(mem));

    char c;

    while( (c = stream_c(_stream)) )
	printf("char: %c\n",c);

    for(int i = 0; i < 100; i++)
	assert(stream_c(_stream) == '\0');
}

int main(int argc, char * argv[])
{
    if(argc < 2)
    {
	fprintf(stderr,"Need a file argument for test\n");
	return 1;
    }

    test_file_stream(argv[1]);
    test_mem_stream();
}
