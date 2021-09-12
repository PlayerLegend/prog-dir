#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef struct http_stream http_stream;
typedef struct http_get http_get;

http_stream * http_stream_open (const char * host, const char * port);

http_get * http_get_open (http_stream * stream, const char * path);
bool http_get_update (buffer_unsigned_char * output, http_get * get);
bool http_stream_has_error (http_stream * stream);
