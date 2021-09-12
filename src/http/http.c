#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"
#include "http.h"
#include "../log/log.h"
#include "../network/network.h"

#define HTTP_VERSION "1.1"

#define CRLF "\r\n"
#define CRLF_LEN 2

#define LINE_END CRLF
#define LINE_END_LEN CRLF_LEN

#define HEADER_END CRLF CRLF
#define HEADER_END_LEN (2 * CRLF_LEN)

struct http_stream {
    int fd;
    buffer_char buffer;
    char * host;
    char * port;
    bool error;
};

struct http_get {
    http_stream * stream;
    bool chunked_encoding;
    bool deflate_encoding;
    bool compress_encoding;
    bool gzip_encoding;
    unsigned long int file_size;
    unsigned int chunk_size;
};

