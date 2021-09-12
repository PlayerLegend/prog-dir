#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef struct http_stream http_stream;
typedef struct http_get http_get;

http_stream * http_stream_open (const char * host, const char * port);

http_get * http_get_open (http_stream * stream, const char * path);
bool http_get_update (http_get * get);
bool http_get_contents (range_const_char * contents, http_get * get);
void http_get_truncate (http_get * get);

keyargs_declare(void,http_parse_url,const char ** protocol; const char ** host; const char ** port; const char ** path; char * url;);
#define http_parse_url(...) keyargs_call(http_parse_url, __VA_ARGS__)
