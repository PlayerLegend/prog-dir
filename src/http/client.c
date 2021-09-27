#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/string.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../chain-io/common.h"
#include "../chain-io/read.h"
#include "../chain-io/fd/read.h"
#include "../log/log.h"
#include "../network/network.h"
#include "../libc/string.h"
#include "../libc/string-extensions.h"

#define HTTP_VERSION "1.1"

#define CRLF "\r\n"
#define CRLF_LEN 2

typedef struct {
    chain_read * fd_read;
    char * host;
    char * port;
}
    http_client;

typedef struct {
    http_client * client;
    enum {
	TRANSFER_ENCODING_IDENTITY,
	TRANSFER_ENCODING_CHUNKED,
    }
	transfer_encoding_type;

    size_t body_size;
    size_t have_size;
    
    buffer_unsigned_char decode;
}
    client_get_arg;

typedef struct {
    bool error;
    bool incomplete;
    range_const_char line;
}
    http_getline_status;

static void http_getline (http_getline_status * status, chain_read * read)
{
    range_const_unsigned_char contents;

    if (!chain_pull (&contents, read, 0))
    {
	status->error = true;
	status->incomplete = false;
	return;
    }

    status->error = false;

    size_t contents_size = range_count (contents);

    if (!contents_size)
    {
	status->incomplete = true;
	return;
    }
    
    range_const_char line_end_sequence = { .begin = CRLF, .end = line_end_sequence.begin + CRLF_LEN };
    size_t line_end_point = range_strstr (&contents.char_cast.const_cast, &line_end_sequence);

    if (line_end_point == contents_size)
    {
	status->incomplete = true;
	return;
    }
    else
    {
	status->incomplete = false;
	status->line.begin = contents.char_cast.const_cast.begin;
	status->line.end = status->line.begin + line_end_point;
	//log_debug ("releasing %zu/%zu from %p", range_count (status->line) + CRLF_LEN, range_count (contents), read);
	chain_release (read, range_count (status->line) + CRLF_LEN);

	//log_debug ("line: %.*s", range_count(status->line), status->line.begin);
    }
}

static void skip_isspace (bool pred, const char ** begin, const char * max)
{
    while (*begin < max)
    {
	if (pred != (bool) isspace (**begin))
	{
	    return;
	}

	(*begin)++;
    }
}

static void split_c (range_const_char * before, range_const_char * after, char c, const range_const_char * input)
{
    size_t sep = range_strchr (input, c);

    before->begin = input->begin;
    before->end = input->begin + sep;
    after->begin = before->end + 1;
    after->end = input->end;
    if (after->begin > after->end)
    {
	after->begin = after->end;
    }
}

static bool parse_http_status (size_t * status, const range_const_char * line)
{
    range_const_char prefix, suffix;

    //log_debug ("status line: [%.*s]", range_count(*line), line->begin);

    split_c (&prefix, &suffix, ' ', line);

    if (!range_streq_string (&prefix, "HTTP/" HTTP_VERSION))
    {
	return false;
    }

    if (0 == range_atozd (status, &suffix))
    {
	return false;
    }

    return true;
}

static bool parse_http_line (client_get_arg * arg, const range_const_char * line)
{
    range_const_char prefix, suffix;

    split_c (&prefix, &suffix, ':', line);

    skip_isspace (true, &suffix.begin, suffix.end);

    if (range_streq_string (&prefix, "Content-Length"))
    {
	if (arg->body_size)
	{
	    log_fatal ("Repeated Content-Length in http header");
	}

	if (arg->transfer_encoding_type != TRANSFER_ENCODING_IDENTITY)
	{
	    log_fatal ("Content-Length applied to non-identity transfer encoding");
	}
	
	if (0 == range_atozd (&arg->body_size, &suffix))
	{
	    log_fatal ("Non-numerical argument to Content-Length: %.*s", range_count(suffix), suffix.begin);
	}
    }
    else if (range_streq_string (&prefix, "Transfer-Encoding"))
    {
	if (range_streq_string(&suffix, "chunked"))
	{
	    if (arg->body_size)
	    {
		log_fatal ("Content-Length set for chunked encoding");
	    }
	    
	    if (arg->transfer_encoding_type != TRANSFER_ENCODING_IDENTITY)
	    {
		log_fatal ("Repeated Transfer-Encoding");
	    }

	    arg->transfer_encoding_type = TRANSFER_ENCODING_CHUNKED;
	}
	else if (range_streq_string(&suffix, "identity") && arg->transfer_encoding_type != TRANSFER_ENCODING_IDENTITY)
	{
	    log_fatal ("Repeated Transfer-Encoding");
	}
	else
	{
	    log_fatal ("Unsupported Transfer-Encoding: %.*s", range_count(suffix), suffix.begin);
	}
    }
    
    return true;
    
fail:
    return false;
}

static chain_status http_get_contents_chunked_header_func (chain_read_interface * interface, void * arg_orig);
static chain_status http_get_body_func (chain_read_interface * interface, void * arg_orig)
{
    client_get_arg * arg = arg_orig;

    if (arg->body_size == 0)
    {
	chain_release(arg->client->fd_read, CRLF_LEN);
	return CHAIN_COMPLETE;
    }

    range_const_unsigned_char contents;

    size_t max_pull_size = 1e6;
    assert (arg->body_size >= arg->have_size);
    size_t want_size = arg->body_size - arg->have_size;
    size_t pull_size = want_size < max_pull_size ? want_size : max_pull_size;

    if (!chain_pull (&contents, arg->client->fd_read, pull_size))
    {
	return CHAIN_ERROR;
    }

    size_t got_size = range_count (contents);

    if (got_size > want_size)
    {
	got_size = want_size;
    }

    arg->have_size += got_size;

    buffer_append_n (interface->buffer, contents.begin, got_size);

    chain_release (arg->client->fd_read, got_size);

    assert (arg->have_size <= arg->body_size);
    
    if (arg->have_size == arg->body_size)
    {
	if (arg->transfer_encoding_type == TRANSFER_ENCODING_CHUNKED)
	{
	    chain_release (arg->client->fd_read, CRLF_LEN);
	    interface->update = http_get_contents_chunked_header_func;
	    return CHAIN_INCOMPLETE;
	}
	else
	{
	    assert (arg->transfer_encoding_type == TRANSFER_ENCODING_IDENTITY);
	    return CHAIN_COMPLETE;
	}
    }
    else
    {
	return CHAIN_INCOMPLETE;
    }
}

bool to_hex (size_t * n, const range_const_char * line)
{
    const char * i;

    *n = 0;

    for_range (i, *line)
    {
	if (*i >= '0' && *i <= '9')
	{
	    *n = (*i - '0') + (*n) * 16;
	}
	else if ( (*i >= 'a' && *i <= 'f') || (*i >= 'A' && *i <= 'F') )
	{
	    *n = (tolower(*i) - 'a') + 10 + (*n) * 16;
	}
	else
	{
	    break;
	}
    }

    return range_index (i, *line) != 0;
}

static chain_status http_get_contents_chunked_header_func (chain_read_interface * interface, void * arg_orig)
{
    client_get_arg * arg = arg_orig;
    http_getline_status line_status;
    
    http_getline (&line_status, arg->client->fd_read);

    if (line_status.error)
    {
	log_fatal ("Could not read chunk header from server");
    }
    
    if (line_status.incomplete)
    {
	return CHAIN_INCOMPLETE;
    }

    if (!to_hex (&arg->body_size, &line_status.line))
    {
	log_fatal ("Couldn't parse hex number");
    }
    
    arg->have_size = 0;

    interface->update = http_get_body_func;

    return CHAIN_INCOMPLETE;
    
fail:
    return CHAIN_ERROR;
}

static chain_status http_get_header_func (chain_read_interface * interface, void * arg_orig)
{
    client_get_arg * arg = arg_orig;
    http_getline_status line_status;
    
    http_getline (&line_status, arg->client->fd_read);

    if (line_status.error)
    {
	log_fatal ("Could not read from server");
    }

    if (line_status.incomplete)
    {
	return CHAIN_INCOMPLETE;
    }

    if (range_is_empty (line_status.line))
    {
	switch (arg->transfer_encoding_type)
	{
	case TRANSFER_ENCODING_IDENTITY:
	    interface->update = http_get_body_func;
	    break;
	    
	case TRANSFER_ENCODING_CHUNKED:
	    interface->update = http_get_contents_chunked_header_func;
	    break;
	    
	default:
	    log_fatal ("Invalid transfer encoding type %zu", arg->transfer_encoding_type);
	    break;
	}
    }
    else if (!parse_http_line (arg, &line_status.line))
    {
	log_fatal ("Failed to parse header line: %.*s", (int)range_count(line_status.line), line_status.line.begin);
    }

    return CHAIN_INCOMPLETE;

fail:
    return CHAIN_ERROR;
}

static chain_status http_get_status_func (chain_read_interface * interface, void * arg_orig)
{
    client_get_arg * arg = arg_orig;
    http_getline_status line_status;
    
    http_getline (&line_status, arg->client->fd_read);

    if (line_status.error)
    {
	log_fatal ("Could not read from server");
    }

    if (line_status.incomplete)
    {
	return CHAIN_INCOMPLETE;
    }

    /*if (range_is_empty (line_status.line))
    {
	return CHAIN_INCOMPLETE;
	}*/

    size_t status;

    if (!parse_http_status (&status, &line_status.line))
    {
	log_fatal ("Server returned malformed status line");
    }

    if (status != 200)
    {
	log_fatal ("Server returned error status %zu", status);
    }

    interface->update = http_get_header_func;

    return CHAIN_INCOMPLETE;
    
fail:
	return CHAIN_ERROR;
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

chain_read * http_client_get (http_client * client, const char * path)
{
    if (!get_request (chain_read_fd (client->fd_read), client->host, client->port, path))
    {
	log_fatal ("Failed to message server");
    }
    
    chain_read * retval = chain_read_new (sizeof(client_get_arg));

    *chain_read_access_interface(retval) = (chain_read_interface)
    {
	.update = http_get_status_func,
    };
    
    *(client_get_arg*) chain_read_access_arg(retval) = (client_get_arg)
    {
	.client = client,
    };

    return retval;

fail:
    return NULL;
}

http_client * http_client_connect (const char * host, const char * port)
{
    int fd = tcp_connect (host, port);

    if (fd < 0)
    {
	perror ("tcp_connect");
	return NULL;
    }
    
    http_client * retval = calloc (1, sizeof (*retval));

    retval->fd_read = chain_read_open_fd(fd);

    retval->host = strdup (host);
    retval->port = strdup (port);

    return retval;
}

void http_client_disconnect (http_client * client)
{
    chain_read_free (client->fd_read);
    free (client->host);
    free (client->port);
    free (client);
}
