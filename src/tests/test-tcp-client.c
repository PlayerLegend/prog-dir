#include "network.h"
#include "print.h"
#include <stdlib.h>

int main(int argc, char * argv[])
{
    if( argc != 4 )
    {
	print_error("usage: %s [host] [service] [message]",argv[0]);
	exit(1);
    }

    char * host = argv[1];
    char * service = argv[2];
    char * message = argv[3];

    FILE * comm;

    if( NULL == (comm = tcp_connect_stream(host,service)) )
    {
	print_error("Failed to connect stream");
	exit(1);
    }
    
    if(ferror(comm))
    {
	perror("comm before");
	exit(1);
    }
    

    fprintf(comm,"%s\n",message);
    fflush(comm);

    if(ferror(comm))
    {
	perror("comm after");
	exit(1);
    }
    
    printf("client sent line: %s\n",message);

    struct { char * text; size_t len; } line = { 0, 0 };

    if( -1 == getline(&line.text,&line.len,comm) )
    {
	perror("getline");
	fclose(comm);
	free(line.text);
	exit(1);
    }

    printf("client got message: %s\n",line.text);
    free(line.text);
    fclose(comm);

    printf("success\n");
    
    return 0;
}
