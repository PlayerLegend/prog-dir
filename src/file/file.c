#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "file.h"

#define STREAM_READ_SIZE (2<<20)
#define STREAM_MAX_SIZE (50*(2<<20))

bool load_file(buffer_char * file_contents, FILE * file)
{
    size_t have_size = 0;
    size_t next_size;
    size_t fread_return;
    while (1)
    {
	next_size = have_size + STREAM_READ_SIZE + 1;
	buffer_resize(*file_contents, next_size);
	fread_return = fread(file_contents->begin + have_size, 1, STREAM_READ_SIZE, file);
	if (fread_return < STREAM_READ_SIZE)
	{
	    file_contents->end = file_contents->begin + have_size + fread_return;
	    *file_contents->end = '\0';
	    return true;
	}
	
	have_size += fread_return;

	if (have_size > STREAM_MAX_SIZE)
	{
	    return false;
	}
    }

    return false;
}
