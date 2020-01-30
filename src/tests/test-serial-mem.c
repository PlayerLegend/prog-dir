#include "serial.h"

int main(int argc, char * argv[])
{
    char buffer[] = "abcdefghijklmnopqrstuvwxyz";
    
    serial read;

    serial_mem(&read,buffer,sizeof(buffer));

    char c;
    
    while( (c = serial_read(read)) )
    {
	printf("char: %c\n",c);
    }

    return 0;
}
