#include <stdio.h>

typedef struct {
    char *begin, *end;
    char *point;
    FILE * file;
}
    stream;

#define stream_mem(_ptr,_size)		\
    (stream){ .begin = (char*)(_ptr), .end = (char*)(_ptr) + _size, .point = (char*)(_ptr)}

#define stream_file_size(_file,_size)			\
    (stream){ .file = (_file), .end = (char*)(_size), .point = (char*)(-1) }

#define stream_file(_file)			\
    stream_file_size(_file,4096)

char _stream_load(stream * stream);

#define stream_c(_stream)			\
    ((_stream).point < (_stream).end ? *(_stream).point++ : _stream_load(&(_stream)))
