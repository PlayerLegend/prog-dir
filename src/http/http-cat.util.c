#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../chain-io/common.h"
#include "../chain-io/read.h"
#include "../chain-io/write.h"
#include "../chain-io/fd/write.h"
#include "client.h"
#include "../log/log.h"

int main (int argc, char * argv[])
{
    if (argc < 3)
    {
	log_fatal ("usage: %s [host] [port] [path]", argv[0]);
    }

    const char * host = argv[1];
    const char * port = argv[2];
    
    http_client * client = http_client_connect (host, port);

    chain_write * fd_out = chain_write_open_fd(STDOUT_FILENO);

    range_const_unsigned_char contents;

    chain_read * get;

    for (int i = 3; i < argc; i++)
    {
	get = http_client_get(client, argv[i]);

        while (chain_pull (&contents, get, 1e6) && chain_push(fd_out, &contents))
	{
	    chain_release (get, range_count (contents));
	}
	
	if (chain_read_is_error(get))
	{
	    log_fatal ("Read error");
	}

	if (chain_write_is_error(fd_out))
	{
	    log_fatal ("Write error");
	}

	chain_read_free (get);
    }
    
    http_client_disconnect(client);

    chain_write_free(fd_out);
    
    return 0;
    
fail:
    return 1;
}
