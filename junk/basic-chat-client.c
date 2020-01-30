#include "network.h"
#include "print.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char * argv[])
{
    if(argc != 3)
    {
	print_error("usage: %s [name] [server] [port]",argv[0]);
	exit(1);
    }

    char * name = argv[1];
    char * node = argv[2];
    char * service = argv[3];

    FILE * connection;

    if( NULL == (connection = tcp_connect_stream(node,service)) )
    {
	exit(1);
    }

    fprintf(connection,"%s\n",name);

    

    while( !ferror(connection) && !feof(connection) )
    {
	
    }
}
