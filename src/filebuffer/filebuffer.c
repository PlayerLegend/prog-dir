#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"
#include "filebuffer.h"

int filebuffer(buffer_char * filename, int in_fd)
{
    char template[] = "filestore-add-XXXXXX";

    int tmp_fd = mkstemp(template);

    if (tmp_fd < 0)
    {
	perror ("mkstemp");
	return -1;
    }

    int size;

    buffer_char file_buffer = {0};
    size_t wrote_size;

    while (0 < (size = buffer_read (.fd = in_fd, .buffer = &file_buffer, .max_buffer_size = BUFFER_IO_SMALL)))
    {
	wrote_size = 0;
	while (0 < (size = buffer_write (.fd = tmp_fd, .buffer = &file_buffer.range_cast.const_cast, .wrote_size = &wrote_size)))
	{	    
	}

	if (size < 0)
	{
	    goto fail;
	}

	buffer_rewrite (file_buffer);
    }

    if (size < 0)
    {
	goto fail;
    }

    free (file_buffer.begin);

    const char * TMPDIR = getenv ("TMPDIR");
    if (!TMPDIR)
    {
	TMPDIR = "/tmp";
    }

    buffer_printf (filename, "%s", template);

    if (0 > lseek (tmp_fd, 0, SEEK_SET))
    {
	perror ("lseek");
	goto fail;
    }
    
    return tmp_fd;

fail:
    free (file_buffer.begin);
    close (tmp_fd);
    return -1;
}
