#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../buffer_io/buffer_io.h"
#endif

typedef struct url url;
struct url {
    char * buffer;
    const char * protocol;
    const char * host;
    const char * port;
    const char * path;
};

typedef struct http_get http_get;
struct http_get {
    buffer_char * output;
    int fd;
    const char * host;
    const char * port;
    const char * path;
};

bool http_parse_url (url * url, char * url_text);

struct http_queue_get_arg { int fd; const char *host, *port, *path; };
bool _http_queue_get (struct http_queue_get_arg arg);
#define http_queue_get(...) _http_queue_get ( (struct http_queue_get_arg){__VA_ARGS__} )

void http_ready_read (int fd);


