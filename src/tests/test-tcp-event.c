#include <string.h>
#include "tcp_event.h"
#include "print.h"
#include <stdlib.h>
#include <assert.h>

bool cb_connect(tcp_event_connection_state * client)
{
    printf("client connected\n");
    *array_push(&client->read.term_bytes) = ';';
    client->read.term_size = 7;
    return true;
}

bool cb_finished_read(tcp_event_connection_state * client)
{
    printf("client said: '%.*s'\n",
	   (int)(client->read.bytes.end - client->read.bytes.begin),
	   client->read.bytes.begin);

    char append[] = "asdf\n";
    array_append_several(&client->write.bytes,append,6);
    client->write.active = true;
    
    return true;
}

bool cb_finished_write(tcp_event_connection_state * client)
{
    printf("server sent: '%.*s'\n",
	   (int)(client->write.bytes.end - client->write.bytes.begin),
	   client->write.bytes.begin);
    array_rewrite(&client->write.bytes);
    return true;
}

void cb_disconnect(tcp_event_connection_state * client)
{
    printf("client disconnected\n");
}

int main(int argc, char * argv[])
{
    if(argc != 2)
    {
	print_error("usage: %s [port]",argv[0]);
	exit(1);
    }

    tcp_event_config config =
    {
	.connect = cb_connect,
	.finished_read = cb_finished_read,
	.finished_write = cb_finished_write,
	.disconnect = cb_disconnect,
    };

    if( -1 == tcp_event_listen(argv[1],&config) )
    {
	print_error("tcp server failed");
	exit(1);
    }

    return 0;
}
