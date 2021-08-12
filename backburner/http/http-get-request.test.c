#include "client.c"
#include <sys/socket.h>

int main(int argc, char * argv[])
{
    int fd;
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
    return 0;
}
