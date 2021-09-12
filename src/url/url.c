#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "url.h"
#include "../log/log.h"

void get_scheme (buffer_char * scheme, range_const_char * input)
{
    while (!range_is_empty(*input))
    {
	input->begin++;
    }
}

bool url_parse (url * url, const char * input)
{
    const char * end;
    const char * begin = input;

    // scheme
    {
	end = strchr (input, ':');
	if (!end)
	{
	    log_fatal ("No url scheme separator");
	}

	if (end == begin)
	{
	    log_fatal ("Empty url scheme");
	}

	buffer_strncpy (&url->scheme, input, end - input);
	
	begin = end + 1;
    }

    if (begin[0] == '/' && begin[1] == '/') //authority
    {
	const char * user_host_sep = strchr (begin, '@');
	const char * host_port_sep = strchr (user_host_sep ? user_host_sep : begin, ':');
	const char * path_start = strchr (host_port_sep ? host_port_sep : user_host_sep ? user_host_sep : begin, '/');

	const char * host_begin = user_host_sep ? user_host_sep + 1 : begin;
	const char * host_end = host_port_sep ? host_port_sep : path_start;

	if (host_begin > begin)
	{
	    buffer_strncpy (&url->userinfo, begin, host_begin - begin - 1);
	}
	else
	{
	    buffer_strcpy (&url->userinfo, "");
	}

	if ()
	{
	    buffer_strncpy(&url->port, host_port_sep + 1, )
	}
	
	if (end)
	{
	    buffer_strncpy (&url->userinfo, begin, begin - end);
	    begin = end + 1;
	}
	else
	{
	    buffer_strcpy (&url->userinfo, "");
	}

	end = strchr (begin, ':');

	if (end)
	{
	    buffer_strncpy (&url->host, begin, begin - end);
	    begin = end + 1;
	}
	else
	{
	    buffer_strcpy (&url->host, "");
	}

	end = strchr (begin, '/');

	if (end)
	{
	    buffer_strncpy (&url->port, begin, begin - end);
	}
	else
	{
	    buffer_strncpy (&url->
	}
    }
    else
    {
	
    }

fail:
    return false;
}

void url_clear (url * url);
