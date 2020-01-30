#include "serial.h"

int main(int argc, char * argv[])
{
    if( argc != 2 )
	return 1;

    const char * file_name = argv[1];

    FILE * file = fopen(file_name,"r");

    if(file)
    {
	serial read;
	
	serial_file(&read,file);
	
	char c;
	
	while( (c = serial_read(read)) )
	{
	    printf("char: %c\n",c);
	}
	
	fclose(file);
    }
    else
    {
	perror(__func__);
    }

    return 0;
}
