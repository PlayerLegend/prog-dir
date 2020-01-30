#include "network.h"
#include "print.h"
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
    if(argc != 3)
    {
	print_error("usage: %s [service] [message]",argv[0]);
	exit(1);
    }

    char * service = argv[1];
    char * message = argv[2];

    int listen;

    if( -1 == (listen = tcp_host(service)) )
	exit(1);

    FILE * comm;

    if( NULL == (comm = tcp_listen_stream(listen)) )
    {
	print_error("failed to listen");
	close(listen);
	exit(1);
    }

    close(listen);
	
    struct { char * text; size_t len; } line = { 0, 0 };

    if( -1 == getline(&line.text,&line.len,comm) )
    {
	perror("getline");
	fclose(comm);
	free(line.text);
	exit(1);
    }

    printf("server read line: %s\n",line.text);

    free(line.text);

    fprintf(comm,"%s\n",message);
    fflush(comm);

    printf("server sent line: %s\n",message);

    fclose(comm);

    printf("success\n");
    
    return 0;
}
