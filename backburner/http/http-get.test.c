#include "client.c"
#include <sys/socket.h>

int main(int argc, char * argv[])
{
    buffer_char buffer = {};
    url url;
    int fd;
    for (int i = 1; i < argc; i++)
    {
	//log_normal ("%s:", argv[i]);
	if (!http_parse_url (&url, argv[i]))
	{
	    log_error ("Failed to parse url");
	    return 1;
	}
	
	fd = tcp_connect (url.host, url.port);

	if (fd <= 0)
	{
	    log_error ("Couldn't connect");
	    return 1;
	}

	if (!http_get (.output = &buffer, .fd = fd, .host = url.host, .port = url.port, .path = url.path))
	{
	    log_error ("Failed to get");
	    return 1;
	}
	
	printf("%s", buffer.begin);
    }
    /*int fd;
    buffer_char buffer = {};
    for (int i = 1; i < argc; i++)
    {
	fd = http_get (argv[i]);
	shutdown (fd, 1);
	int got;
	while (0 < (got = buffer_read (.buffer = &buffer, .fd = fd)))
	{}
	printf("%s\n", buffer.begin);
    }
    free (buffer.begin);
    return 0;*/

    return 0;
}
