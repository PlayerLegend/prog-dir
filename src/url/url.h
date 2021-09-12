#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef struct {
    buffer_char scheme, userinfo, host, port, path, query, fragment;
}
    url;

bool url_parse (url * url, const char * input);
void url_clear (url * url);
