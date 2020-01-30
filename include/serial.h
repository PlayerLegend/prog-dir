#include <stdio.h>

typedef struct
{
    FILE * file;
    struct
    {
	char * text;
	int size;
	int read;
	int index;
    }
	buffer;
}
    serial;

char
_serial_reload(serial * reload);

#define serial_read(serial) \
    ( (serial).buffer.index < (serial).buffer.read ? (serial).buffer.text[(serial).buffer.index++] : _serial_reload(&(serial)) )

void
serial_file(serial * open, FILE * file);

void
serial_mem(serial * open, void * mem, size_t size);
