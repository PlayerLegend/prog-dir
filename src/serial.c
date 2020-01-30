#include "serial.h"
#include "stdlib.h"

char
_serial_reload(serial * reload)
{
    if(reload->file)
    {
	if(reload->buffer.size == 0)
	{
	    fprintf(stderr,"%s: buffer size is 0, cannot allocate memory for read", __func__);
	    goto shutdown;
	}

	if(reload->buffer.text == NULL)
	{
	    reload->buffer.text = malloc(reload->buffer.size);
	    if( reload->buffer.text == NULL )
	    {
		perror(__func__);
		goto shutdown;
	    }
	}

	if( feof(reload->file) )
	    goto shutdown;
	
	reload->buffer.read = fread(reload->buffer.text,1,reload->buffer.size,reload->file);
	
	if( ferror(reload->file) )
	{
	    perror(__func__);
	    goto shutdown;
	}

	reload->buffer.index = 1;
	return reload->buffer.text[0];
    }
    
    return '\0';

shutdown:
    free(reload->buffer.text);
    *reload = (serial){ 0 };
    return '\0';
}

void
serial_file(serial * open, FILE * file)
{
    size_t buffer_size = 1024;
    
    *open = (serial)
    {
	.file = file,
	.buffer.size = buffer_size,
    };
}

void
serial_mem(serial * open, void * mem, size_t size)
{
    *open = (serial)
    {
	.buffer.text = mem,
	.buffer.read = size,
    };
}
