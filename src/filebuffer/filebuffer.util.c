#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"
#include "filebuffer.h"

int main()
{
    buffer_char filename = {0};
    int fd = filebuffer (&filename, STDIN_FILENO);

    if (fd < 0)
    {
	abort();
    }

    int size;

    buffer_char write_buffer = {0};
    size_t wrote_size;
    
    while (0 < (size = buffer_read (.fd = fd, .buffer = &write_buffer, .max_buffer_size = BUFFER_IO_SMALL)))
    {
	wrote_size = 0;
	while (0 < (size = buffer_write (.fd = STDOUT_FILENO, .buffer = &write_buffer, .wrote_size = &wrote_size)))
	{	    
	}

	if (size < 0)
	{
	    remove (filename.begin);
	    
	    abort();
	}

	buffer_rewrite (write_buffer);
    }

    remove (filename.begin);
    free (write_buffer.begin);
    free (filename.begin);

    return 0;
}
