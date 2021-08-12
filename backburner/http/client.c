#define _XOPEN_SOURCE 700 // for dprintf
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../buffer_io/buffer_io.h"
#include "client.h"
#include "../network/network.h"
#include "../log/log.h"
#include <sys/socket.h> // shutdown()

bool http_parse_url (url * url, char * url_text)
{
    assert (url_text);
    
    char * end;
    url->protocol = url_text;
    end = strstr (url->protocol, "://");

    if (!end)
    {
	log_error ("Url doesn't specify protocol");
	return false;
    }
    
    *end = '\0';
    url->host = end + 3;
    char * port = strchr (url->host, ':');
    url->port = port;
    end = strchr (url->host, '/');

    if (url->port)
    {
	if (end && url->port < end)
	{
	    *port = '\0';
	    url->port++;
	}
	else
	{
	    url->port = NULL;
	}
    }

    if (end)
    {
	*end = '\0';
	url->path = end + 1;
    }
    else
    {
	url->path = NULL;
    }

    return true;
}

bool _http_queue_get (struct http_queue_get_arg arg)
{
    if (!arg.path || arg.path[0] == '\0')
    {
	return false;
    }
    
    static const char * user_agent = "http-lib/0.0.1";
    static const char * accept = "*/*";

    int result = -1;
    
    if (arg.port)
    {
	result = dprintf (arg.fd, "GET /%s HTTP/1.1\nHost: %s:%s\nUser-Agent: %s\nAccept: %s\n\n", arg.path[0] == '/' ? arg.path + 1 : arg.path, arg.host, arg.port, user_agent, accept);
    }
    else
    {
	result = dprintf (arg.fd, "GET /%s HTTP/1.1\nHost: %s\nUser-Agent: %s\nAccept: %s\n\n", arg.path[0] == '/' ? arg.path + 1 : arg.path, arg.host, user_agent, accept);
    }

    return result > 0;
}

void http_ready_read (int fd)
{
    shutdown (fd, 1);
}

int _http_create_request_multi (const char * host, const char * port, const char * paths[])
{
    int fd = tcp_connect (host, port);
    char ** path = paths;
    while (*path)
    {
	
	if (port)
	{
	
	    buffer_printf (get.output,
			   "GET /%s HTTP/1.1\nHost: %s:%s\nUser-Agent: %s\nAccept: %s\n\n",
			   get.path ? get.path : "",
			   get.host, get.port,
			   user_agent,
			   accept);
	}
	else
	{
	    buffer_printf (get.output,
			   "GET /%s HTTP/1.1\nHost: %s\n User-Agent: %s\nAccept: %s\n\n",
			   get.path ? get.path : "",
			   get.host,
			   user_agent,
			   accept);
	}

    }
}

typedef struct http_get_info http_get_info;

struct http_get_info
{
    char code[3];
    size_t content_length;
};

typedef enum http_get_header_id http_get_header_id;
enum http_get_header_id
{
    HTTP_GET_INVALID,
    HTTP_GET_CONTENT_LENGTH,
};

static http_get_header_id http_identify_header(const char ** line_value, const char * line)
{
    typedef struct pair pair;
    struct pair
    {
	const char * name;
	http_get_header_id id;
    };
    pair list[] =
	{
	    { "Content-Length", HTTP_GET_CONTENT_LENGTH },
	    {},
	};

    pair * iter;
    const char * name_end;
    size_t name_len;
    
    name_end = strchr (line, ':');

    if (name_end <= line)
    {
	return HTTP_GET_INVALID;
    }

    name_len = name_end - line;
    *line_value = name_end + 1;
	
    for (iter = list; iter->name != NULL; iter++)
    {
	if (name_len == strlen(iter->name) && 0 == strncmp (iter->name, line, name_len))
	{
	    return iter->id;
	}
    }

    return HTTP_GET_INVALID;
}

void http_parse_get_header (http_get_info * info, const char * input)
{
    range_const_char line = { .begin = input, .end = input };
    line.end = strchr (input, '\n');
    const char * line_value;

    log_debug("parsing get header");
    
    while (line.end)
    {
	log_debug("header: %s", line.begin);
	switch (http_identify_header (&line_value, line.begin))
	{
	case HTTP_GET_CONTENT_LENGTH:
	    info->content_length = atoi (line_value);
	    log_debug ("content length: %zd", info->content_length);
	    break;
	    
	default:
	case HTTP_GET_INVALID:
	    break;
	}
	
	line.begin = line.end + 1;
	line.end = strchr(line.begin, '\n');
    }
    
    log_debug("done parsing get header");
}

bool _http_get (http_get get)
{
    buffer_rewrite (*get.output);
    
    const char * user_agent = "http-lib/0.0.1";
    const char * accept = "*/*";

    if (get.port)
    {
        buffer_printf (get.output,
		 "GET /%s HTTP/1.1\nHost: %s:%s\nUser-Agent: %s\nAccept: %s\n\n",
		 get.path ? get.path : "",
		 get.host, get.port,
		 user_agent,
		 accept);
    }
    else
    {
        buffer_printf (get.output,
		 "GET /%s HTTP/1.1\nHost: %s\n User-Agent: %s\nAccept: %s\n\n",
		 get.path ? get.path : "",
		 get.host,
		 user_agent,
		 accept);
    }

    long int size;
    size_t wrote_size = 0;

    while ( 0 < (size = buffer_write (.buffer = get.output, .wrote_size = &wrote_size, .fd = get.fd)))
    {}

    if (size != 0)
    {
	perror ("_http_get buffer_write");
	return false;
    }

    buffer_rewrite (*get.output);

    size_t last_header_size = 0;
    size_t header_size = 64;
    char * header_end;

    http_get_info get_info = {0};

    while ( 0 < (size = buffer_read (.buffer = get.output, .max_buffer_size = header_size, .fd = get.fd)))
    {
	header_end = strstr (get.output->begin + last_header_size, "\n\n");

	if (header_end)
	{
	    header_end[1] = '\0';
	    http_parse_get_header (&get_info, get.output->begin);
	    header_end[1] = '\n';
	    char * rest_lower = header_end + 1;
	    char * rest_upper = get.output->end;
	    memmove(get.output->begin, rest_lower, rest_upper - rest_lower);
	    get.output->end = get.output->begin + (rest_upper - rest_lower);
	    break;
	}
	
	last_header_size = header_size;
	header_size = header_size * 2;
    }

    log_debug("shutting down fd for writing");
    shutdown (get.fd, 1);
    log_debug ("now reading content");

    while ( 0 < (size = buffer_read (.buffer = get.output, .max_buffer_size = get_info.content_length, .fd = get.fd)))
    { }
    
    log_debug ("done reading content");

    if (range_count (*get.output) != (ssize_t)get_info.content_length)
    {
	log_error ("Got content length %zu doesn't match expected length %zu", range_count (*get.output), (ssize_t)get_info.content_length);
	return false;
    }
    
    return true;
}

