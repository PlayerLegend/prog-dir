#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "json.h"
#include "../array/buffer.h"
#include "../log/log.h"
#include "../list/list.h"

#include "../table2/table.h"
#define table_string_value json_value value
#include "../table2/table-string.h"

struct json_object {
    table_string map;
};

static bool _skip_whitespace (range_const_char * text)
{
    while (true)
    {
	if (text->begin == text->end)
	{
	    return false;
	}

	if (isspace(*text->begin))
	{
	    text->begin++;
	    continue;
	}

	return true;
    }

    return false;
}

static json_type _identify_next (range_const_char * text)
{
    if (!_skip_whitespace (text))
    {
	return JSON_BADTYPE;
    }
    
    char first_c = *text->begin;
    switch (first_c)
    {
    case '"':
	return JSON_STRING;

    case '{':
	return JSON_OBJECT;

    case '[':
	return JSON_ARRAY;

    case 't':
	return JSON_TRUE;

    case 'f':
	return JSON_FALSE;

    case 'n':
	return JSON_NULL;

    default:
	if (first_c == '-' || ('0' <= first_c && first_c <= '9'))
	{
	    return JSON_NUMBER;
	}
    }

    return JSON_BADTYPE;
}

static bool _read_number (double * output, range_const_char * text)
{
    char * endptr;

    *output = strtod (text->begin, &endptr);

    if (endptr == text->begin)
    {
	return false;
    }

    text->begin = endptr;
    
    return true;
}

static bool _read_string (buffer_char * string, range_const_char * text)
{
    assert (*text->begin == '"');

    text->begin++;

    bool escape = false;
    char add_c;

    buffer_rewrite (*string);

    while (text->begin < text->end)
    {
	if (*text->begin == '\\')
	{
	    escape = true;
	    goto next;
	}

	if (escape)
	{
	    escape = false;
	    assert(text->begin[-1] == '\\');
	    switch (*text->begin)
	    {
	    case '"':
		add_c = '"';
		goto add_c;

	    case '\\':
		add_c = '\\';
		goto add_c;

	    case '/':
		add_c = '/';
		goto add_c;

	    case 'b':
		add_c = '\b';
		goto add_c;

	    case 'f':
		add_c = '\f';
		goto add_c;

	    case 'n':
		add_c = '\n';
		goto add_c;

	    case 'r':
		add_c = '\r';
		goto add_c;

	    case 't':
		add_c = '\t';
		goto add_c;

	    case 'u':
		log_error ("u-hex characters are currently unsupported");
		return false;

	    default:
		log_error ("unrecognized escape code in string (%c): %.*s", *text->begin, range_count(*text) - 1, text->begin);
		return false;
	    }
	}
	else if (*text->begin == '"')
	{
	    *buffer_push (*string) = '\0';
	    text->begin++;
	    return true;
	}
	else
	{
	    add_c = *text->begin;
	    goto add_c;
	}

    add_c:
	*buffer_push (*string) = add_c;
	
    next:
	text->begin++;
    }

    log_error ("file ended while reading string");
    
    return false;
}

static bool _skip_string (range_const_char * text, const char * string)
{
    int len = strlen (string);
    if (range_count (*text) < len + 1)
    {
	return false;
    }

    if (0 != strncmp (text->begin, string, len))
    {
	return false;
    }

    text->begin += len;

    return true;
}

static bool _read_array (json_array * array, range_const_char * text, buffer_char * buffer);
static json_object * _read_object (range_const_char * text, buffer_char * buffer);

static bool _read_value (json_value * value, range_const_char * text, buffer_char * buffer)
{
    *value = (json_value){0};
    
    value->type = _identify_next (text);
    //log_normal ("Read %s", json_type_name(value->type));

    switch (value->type)
    {
    case JSON_OBJECT:
	value->object = _read_object (text, buffer);
	if (!value->object)
	{
	    return false;
	}
	return true;

    case JSON_STRING:
	if (!_read_string(buffer, text))
	{
	    return false;
	}
	value->string = malloc (strlen(buffer->begin) + 1);
	if (!value->string)
	{
	    perror ("malloc");
	    return false;
	}
	strcpy (value->string, buffer->begin);
	return true;

    case JSON_ARRAY:
	if (!_read_array (&value->array, text, buffer))
	{
	    return false;
	}
	return true;

    case JSON_NUMBER:
	if (!_read_number(&value->number, text))
	{
	    return false;
	}
	return true;

    case JSON_FALSE:
	return _skip_string (text, "false");
	
    case JSON_TRUE:
	return _skip_string (text, "true");
	
    case JSON_NULL:
	return _skip_string (text, "null");

    default:
    case JSON_BADTYPE:
	return false;
    }

    return false;
}

static bool _read_array (json_array * array, range_const_char * text, buffer_char * buffer)
{
    struct buffer (json_value) build_array = {};
    
    assert (*text->begin == '[');
    text->begin++;

    bool passed_comma = false;

    while (true)
    {
	_skip_whitespace (text);

	if (*text->begin == ']')
	{
	    if (passed_comma)
	    {
		log_error ("Hanging comma in array: %s", text->begin);
		goto fail;
	    }
	    else
	    {
		goto success;
	    }
	}

	if (!_read_value (buffer_push (build_array), text, buffer))
	{
	    log_error ("Failed to read a value in the array: %s", text->begin);
	    goto fail;
	}

	passed_comma = false;

	_skip_whitespace (text);

	if (*text->begin == ',')
	{
	    text->begin++;
	    passed_comma = true;
	}
    }

fail:
    array->begin = build_array.begin;
    array->end = build_array.end;
    return false;
    
success:
    text->begin++;
    array->begin = build_array.begin;
    array->end = build_array.end;
    return true;
}

static void _free_object (json_object * object)
{
    
}

static json_object * _read_object (range_const_char * text, buffer_char * buffer)
{
    json_object * object = calloc (1, sizeof(*object));
    
    assert (*text->begin == '{');

    text->begin++;

    json_value * set_value;

    bool expect_pair = false;

    while (true)
    {
	if (_identify_next (text) != JSON_STRING)
	{
	    if (*text->begin == '}')
	    {
		if (expect_pair)
		{
		    goto fail;
		}
		else
		{
		    goto success;
		}
	    }
	    else
	    {
		log_error ("Object key is type %s, it should be a string\n%s", json_type_name (_identify_next(text)), text->begin);
		goto fail;
	    }
	}
	
	if (!_read_string (buffer, text))
	{
	    log_error ("JSON object key is not a string: %s", text->begin);
	    goto fail;
	}

	_skip_whitespace (text);

	if (*text->begin != ':')
	{
	    log_error ("Pair separator is missing within JSON object: %s", text->begin);
	    goto fail;
	}

	text->begin++;
	
	set_value = &table_string_include(object->map, buffer->begin)->value.value;

	if (!_read_value (set_value, text, buffer))
	{
	    goto fail;
	}

	expect_pair = false;

	_skip_whitespace (text);

	if (*text->begin == ',')
	{
	    expect_pair = true;
	    text->begin++;
	}
    }
    
fail:
    _free_object (object);
    return NULL;

success:
    text->begin++;
    return object;
}

json_value * json_parse (const char * begin, const char * end)
{
    range_const_char text = { .begin = begin, .end = end };
    buffer_char buffer = {0};

    json_value * value = calloc (1, sizeof(*value));

    if (!_read_value (value, &text, &buffer))
    {
	json_free (value);
	return NULL;
    }

    return value;
}

json_value * json_lookup (const json_object * object, const char * key)
{
    table_string_query query = table_string_query(key);
    table_string_bucket bucket = table_string_bucket(object->map, query);

    table_string_match(bucket, query);

    table_string_item * item = table_get_bucket_item (bucket);

    return item ? &item->value.value : NULL;
}

void json_free (json_value * value)
{
    json_value * member;
    
    switch (value->type)
    {
    case JSON_ARRAY:
	for_range(member, value->array)
	{
	    json_free (member);
	}
	break;

    case JSON_BADTYPE:
    case JSON_FALSE:
    case JSON_NULL:
    case JSON_TRUE:
    case JSON_NUMBER:
	break;

    case JSON_STRING:
	free (value->string);
	break;

    case JSON_OBJECT:
	_free_object (value->object);
	break;

    }

    free (value);
}

const char * json_type_name(json_type type)
{
    static const char * _name[] = { "null", "number", "true", "false", "string", "array", "object", "badtype" };
    return _name[type];
}

typedef struct json_number_options json_number_options;
struct json_number_options {
    double null_value;
};

double _json_get_number(json_get_number_arg arg)
{
    json_value * value = json_lookup(arg.parent, arg.key);

    if (value->type == JSON_NULL)
    {
	if (arg.optional)
	{
	    return arg.default_value;
	}
	
	log_error ("Object has no child %s", arg.key);
	goto fail;
    }

    if (value->type != JSON_NUMBER)
    {
	log_error ("Object child %s is not a number", arg.key);
	goto fail;
    }

    return value->number;

fail:
    if (arg.success)
    {
	*arg.success = false;
    }
    return 0;
}

const char * _json_get_string(json_get_string_arg arg)
{
    json_value * value = json_lookup(arg.parent, arg.key);

    if (value->type == JSON_NULL)
    {
	if (arg.optional && arg.default_value)
	{
	    return arg.default_value;
	}
	
	log_error ("Object has no child %s", arg.key);
	goto fail;
    }

    if (value->type != JSON_STRING)
    {
	log_error ("Object child %s is not a string", arg.key);
	goto fail;
    }

    assert (value->string != NULL);

    return value->string;

fail:
    if (arg.success)
    {
	*arg.success = false;
    }
    
    return NULL;
}

const json_array * _json_get_array(json_get_array_arg arg)
{
    json_value * value = json_lookup(arg.parent, arg.key);

    if (value->type == JSON_NULL)
    {
	log_error ("Object has no child %s", arg.key);
	return NULL;
    }

    if (value->type != JSON_ARRAY)
    {
	log_error ("Object child %s is not an array", arg.key);
	return NULL;
    }

    return &value->array;
}

const json_object * _json_get_object(json_get_object_arg arg)
{
    json_value * value = json_lookup(arg.parent, arg.key);

    if (value->type == JSON_NULL)
    {
	if (!arg.optional)
	{
	    log_error ("Object has no child %s", arg.key);
	}
	return NULL;
    }

    if (value->type != JSON_OBJECT)
    {
	log_error ("Object child %s is not an object", arg.key);
	return NULL;
    }

    return value->object;
}
