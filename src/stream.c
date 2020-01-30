#include "stream.h"

#include <stdlib.h>

#include <assert.h>

char _stream_load(stream * _stream)
{
    if(!_stream->file)
	return '\0';
    
    if(!_stream->begin)
    {
	_stream->begin = malloc((size_t)_stream->end);
	_stream->end += (size_t)_stream->begin;
    }

    assert(_stream->begin != NULL);
    assert(_stream->end != NULL);
    assert(_stream->file != NULL);
    
    _stream->end = _stream->begin + fread(_stream->begin,1,_stream->end - _stream->begin,_stream->file);

    if(ferror(_stream->file))
    {
	perror(__func__);
	goto halt;
    }

    if(_stream->end == _stream->begin)
	goto halt;

    _stream->point = _stream->begin;

    return *_stream->point++;

halt:
    _stream->file = NULL;
    free(_stream->begin);
    _stream->point = (char*)(-1);
    return '\0';
}
