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



#define http_get_getline_init(get) buffer_getline_init( &(get)->stream->buffer )
static void http_get_getline_end (range_char * line, http_get * get)
{
    buffer_getline_end (get->chunked_encoding ? get->file_size : 0, CRLF, line, &get->stream->buffer);
}

keyargs_declare_static(bool,http_get_getline,range_char * line; http_get * get; long int protect_size; bool init;);
#define http_stream_getline(...) keyargs_call(http_get_getline, __VA_ARGS__)
keyargs_define_static(http_get_getline)
{
    bool retval = buffer_getline_fd (.line = args.line, .read.buffer = &args.get->stream->buffer, .read.fd = args.get->stream->fd, .sep = CRLF, .protect_size = args.protect_size, .init = args.init);

    if (!retval)
    {
	return false;
    }

    *args.line->end = '\0';
    
    return true;
}

// http_stream_open

http_stream * http_stream_open (const char * host, const char * port)
{
    http_stream * retval = calloc (1, sizeof (*retval));
    retval->fd = tcp_connect(host, port);
    retval->host = strcpy (malloc (strlen (host) + 1), host);
    retval->port = strcpy (malloc (strlen (port) + 1), port);
    return retval;
}

// http_get_open

static int read_status_line (const char * status)
{
    char * sep = strchr (status, ' ');
    if (!sep)
    {
	log_fatal ("Malformed status line in http get response");
    }

    *sep = '\0';
    sep++;

    const char * want_http = "HTTP";

    if (0 != strncmp (status, want_http, strlen (want_http)))
    {
	log_fatal ("Server response is not an HTTP response");
    }

    return atoi (sep);

fail:
    return -1;
}

static bool parse_header_line (char ** key, char ** value, char * line)
{
    const char * sep = ": ";
    
    char * split = strstr (line, sep);

    if (!split)
    {
	log_fatal ("Malformed line in HTTP header");
    }

    *split = '\0';
    *key = line;
    *value = split + strlen (sep);

    return true;

fail:
    return false;
}

static char * skip_whitespace (char * c)
{
    while (*c && isspace (*c))
    {
	c++;
    }

    return c;
}

static void parse_csv_arg (buffer_string * list, char * arg)
{
    buffer_rewrite (*list);
    
    char * begin = skip_whitespace (arg);
    arg = begin;
    
    while (*arg)
    {
	if (*arg == ',')
	{
	    *arg = '\0';
	    *buffer_push (*list) = begin;
	    begin = skip_whitespace (arg + 1);
	    arg = begin;
	}
	arg++;
    }

    *buffer_push (*list) = begin;
}

static bool get_request (int fd, const char * host, const char * port, const char * path)
{
    assert (host);
    assert (*host);
    return 0 < dprintf (fd,
			"GET /%s HTTP/" HTTP_VERSION "\n" "Host: %s:%s\n" "User-Agent: http-lib/0.0.1\n" "Accept: */*\n" "\n",
			path,
			host,
			port);
}

http_get * http_get_open (http_stream * stream, const char * path)
{
    http_get * retval = calloc (1, sizeof (*retval));
    buffer_string csv_list = {0};
    retval->stream = stream;
    
    if (!get_request (stream->fd, stream->host, stream->port, path))
    {
	log_fatal ("http_get_open: could not write to server");
    }
    
    range_char line;

    if (!http_stream_getline(.line = &line, .get = retval, .init = true))
    {
	log_fatal ("Could not read a status line from the server");
    }

    int status = read_status_line (line.begin);

    if (status < 0)
    {
	log_fatal ("Server responded with a malformed status line");
    }

    if (status != 200)
    {
	log_fatal ("Server returned error %d", status);
    }

    char * key;
    char * value;

    while (http_stream_getline (.line = &line, .get = retval))
    {
	if (range_is_empty (line))
	{
	    break;
	}
	
	if (!parse_header_line(&key, &value, line.begin))
	{
	    log_fatal ("Failed to parse an HTTP header line");
	}

	if (0 == strcmp (key, "Transfer-Encoding"))
	{
	    parse_csv_arg(&csv_list, value);

	    if (range_is_empty (csv_list))
	    {
		log_fatal ("Transfer-Encoding specified without arguments");
	    }

	    char ** arg;

	    for_range (arg, csv_list)
	    {
		if (0 == strcmp (*arg, "chunked"))
		{
		    retval->chunked_encoding = true;
		}
		else if (0 == strcmp (*arg, "compress"))
		{
		    retval->compress_encoding = true;
		}
		else if (0 == strcmp (*arg, "deflate"))
		{
		    retval->deflate_encoding = true;
		}
		else if (0 == strcmp (*arg, "gzip"))
		{
		    retval->gzip_encoding = true;
		}
		else if (0 == strcmp (*arg, "identity"))
		{
		    retval->chunked_encoding =
			retval->compress_encoding =
			retval->deflate_encoding =
			retval->gzip_encoding = false;
		}
		else
		{
		    log_fatal ("Invalid argument to HTTP header Transfer-Encoding: '%s'", *arg);
		}
	    }
	}
	else if (0 == strcmp (key, "Content-Length"))
	{
	    retval->file_size = atoll (value);
	}
    }

    http_get_getline_end(&line, retval);

    if (retval->compress_encoding)
    {
	log_fatal ("Server specified compress encoding, but it is not implemented");
    }
    
    if (retval->deflate_encoding)
    {
	log_fatal ("Server specified deflate encoding, but it is not implemented");
    }
    
    if (retval->gzip_encoding)
    {
	log_fatal ("Server specified gzip encoding, but it is not implemented");
    }

    if (!retval->chunked_encoding && !retval->file_size)
    {
	log_fatal ("The server did not provide a file size, but did not specify chunked encoding");
    }

    free (csv_list.begin);
    return retval;

fail:
    free (retval);
    free (csv_list.begin);
    return NULL;
}

static bool skip_trailer (http_get * get)
{
    range_char line;

    if (!http_stream_getline(.line = &line,
			    .get = get,
			    .init = true,
			    .protect_size = get->file_size))
    {
	log_fatal ("Chunked get ended without trailer");
    }
    
    if (!range_is_empty (line))
    {
	while (http_stream_getline (.line = &line,
				    .get = get,
				    .protect_size = get->file_size))
	{
	    if (range_is_empty (line))
	    {
		goto success;
	    }
	}

	log_fatal ("Chunked get trailer is incomplete");
    }

success:
    http_get_getline_end(&line, get);
    assert (get->stream->buffer.end == line.begin);
    return true;

fail:
    return false;
}

static bool read_chunk_size (http_get * get)
{
    range_char line;
	
    if (get->file_size > 0)
    {
	if (!http_stream_getline (.line = &line,
				  .get = get,
				  .init = true,
				  .protect_size = get->file_size))
	{
	    log_fatal ("Could not read an empty line between chunks");
	}

	assert (line.begin == get->stream->buffer.begin + get->file_size);

	if (!range_is_empty (line))
	{
	    log_fatal ("Expected an empty line between chunks");
	}
    }

    if (!http_stream_getline(.line = &line,
			     .get = get,
			     .init = !(get->file_size > 0),
			     .protect_size = get->file_size))
    {
	log_fatal ("Could not read a chunk size");
    }

    assert (line.begin == get->stream->buffer.begin + get->file_size + ((get->file_size > 0) ? CRLF_LEN : 0));
   
    if (1 != sscanf (line.begin, "%x", &get->chunk_size))
    {
	log_fatal ("Failed to parse chunk size");
    }

    
    http_get_getline_end(&line, get);
    
    return true;

fail:
    return false;
    
}

bool http_get_update (http_get * get)
{
    if (get->chunked_encoding)
    {   
	if ((size_t)range_count (get->stream->buffer) >= get->file_size)
	{
	    if (!read_chunk_size (get))
	    {
		log_fatal ("Could not read a chunk size");
	    }
	    
	    get->file_size += get->chunk_size;
	}
	
	int retval = buffer_read (.fd = get->stream->fd,
				  .buffer = &get->stream->buffer,
				  .max_buffer_size = get->file_size);
	
	if (retval < 0)
	{
	    log_fatal ("Could not read a chunk");
	}

	if (retval == 0)
	{
	    if (get->chunk_size > 0)
	    {
		if ((size_t)range_count (get->stream->buffer) >= get->file_size)
		{
		    return true;
		}
		
		log_fatal ("HTTP stream ended during chunked transfer");
	    }

	    if (!skip_trailer (get))
	    {
		log_fatal ("Could not skip chunked encoding trailer");
	    }
	}

	return retval != 0;
    }
    else
    {
	return 0 != buffer_read (.fd = get->stream->fd, .buffer = &get->stream->buffer, .max_buffer_size = get->file_size);
    }

fail:
    get->stream->error = true;
    return false;
}

bool http_get_contents (range_const_char * contents, http_get * get)
{
    if (get->stream->error)
    {
	return false;
    }
    
    size_t size = range_count (get->stream->buffer);
    size_t size_max = get->file_size - get->chunk_size;
    assert (get->file_size > get->chunk_size);
    
    if (size > size_max)
    {
	size = size_max;
    }
    
    contents->begin = get->stream->buffer.begin;
    contents->end = contents->begin + size;

    return true;
}

void http_get_truncate (http_get * get)
{
    get->file_size -= range_count (get->stream->buffer);
    buffer_rewrite (get->stream->buffer);
}

keyargs_define(http_parse_url)
{
    const char * protocol = NULL;
    const char * host = NULL;
    const char * port = NULL;
    const char * path = NULL;
    
    char * begin = args.url;
    char * mid = strstr (begin, "://");
    char * mid2;

    if (!mid)
    {
	protocol = "http";
    }
    else
    {
	protocol = begin;
	*mid = '\0';
	begin = mid + 3;
    }

    const char * default_port = strcmp(protocol, "https") == 0 ? "443" : "80";
    
    host = begin;
    mid = strchr (begin, ':');
    mid2 = strchr (begin, '/');

    if (mid && (!mid2 || mid2 > mid))
    {
	*mid = '\0';
	port = mid + 1;
	if (mid2)
	{
	    *mid2 = '\0';
	    path = mid2 + 1;
	}
    }
    else if (mid2)
    {
	port = default_port;
	path = mid2 + 1;
	*mid2 = '\0';
    }
    else
    {
	port = default_port;
	path = "";
    }

    if (args.protocol)
    {
	*args.protocol = protocol;
    }
    
    if (args.host)
    {
	*args.host = host;
    }
    
    if (args.port)
    {
	*args.port = port;
    }
    
    if (args.path)
    {
	*args.path = path;
    }
}
