#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../immutable/immutable.h"
#include "arch.h"
#include "text.h"

bool is_vm_address (const char * text)
{
    if (*text != '@')
    {
	return false;
    }

    while (*(++text))
    {
	if (*text < '0' || *text > '9')
	{
	    return false;
	}
    }

    return true;
}

bool read_vm_address (vm_address * result, const char * text)
{
    if (!is_vm_address (text))
    {
	return false;
    }
    else
    {
	*result = atoi (text + 1);
	return true;
    }
}

bool is_vm_float (const char * text)
{
    while (*(++text))
    {
	if ( (*text) >= '0' && (*text) <= '9' )
	{
	    continue;
	}

	if ( *text == '-' || *text == '.' )
	{
	    continue;
	}

	return false;
    }

    return true;
}

bool read_vm_float (vm_float * result, const char * text)
{
    if (!is_vm_float (text))
    {
	return false;
    }
    else
    {
	*result = atof (text);
	return true;
    }
}

bool is_vm_int (const char * text)
{
    while (*(++text))
    {
	if ( (*text < '0' || *text > '9') && (*text != '-') )
	{
	    return false;
	}
    }

    return true;
}

bool read_vm_int (vm_int * result, const char * text)
{
    if (!is_vm_int (text))
    {
	return false;
    }
    else
    {
	*result = atoi (text);
	return true;
    }
}

bool parse_written_constant (buffer_char * result, const char ** type, keywords * keywords, const char * text)
{
    buffer_resize (*result, sizeof (vm_address));
    if (read_vm_address ((vm_address*) result->begin, text))
    {
	*type = keywords->address;
	return true;
    }

    buffer_resize (*result, sizeof (vm_int));
    if (read_vm_int ((vm_int*) result->begin, text))
    {
	*type = keywords->_int;
	return true;
    }

    buffer_resize (*result, sizeof (vm_float));
    if (read_vm_float ((vm_float*) result->begin, text))
    {
	*type = keywords->_float;
	return true;
    }

    return false;
}

void init_keywords (keywords * result, table_string * table)
{
    result->table = table;

    result->address = immutable_string (table, "address");
    
    result->ifelse = immutable_string (table, "ifelse");
    
    result->_float = immutable_string (table, "float");
    result->_int = immutable_string (table, "int");
    result->name = immutable_string (table, "name");
    
    result->exit = immutable_string (table, "exit");
    
    result->load_int = immutable_string (table, "load-int");
    result->store_int = immutable_string (table, "store-int");

    result->load_float = immutable_string (table, "load-float");
    result->store_float = immutable_string (table, "store-float");

    result->load_char = immutable_string (table, "load-char");
    result->store_char = immutable_string (table, "store-char");

    result->add_int = immutable_string (table, "+int");
    result->add_float = immutable_string (table, "+float");
    
    result->subtract_int = immutable_string (table, "-int");
    result->subtract_float = immutable_string (table, "-float");
    
    result->multiply_int = immutable_string (table, "*int");
    result->multiply_float = immutable_string (table, "*float");
    
    result->divide_int = immutable_string (table, "/int");
    result->divide_float = immutable_string (table, "/float");

    result->sum = immutable_string (table, "+");
}

static bool identify_string_opcode (opcode * result, keywords * keywords, const char * name)
{
    struct match { opcode opcode; const char * name; }
    *match,
    matches[] =
	{
	    { OP_EXIT, keywords->exit },
	    
	    { OP_INT_LOAD, keywords->load_int },
	    { OP_INT_STORE, keywords->store_int },

	    { OP_FLOAT_LOAD, keywords->load_float },
	    { OP_FLOAT_STORE, keywords->store_float },

	    { OP_INT_ADD, keywords->add_int },
	    { OP_FLOAT_ADD, keywords->add_float },
	    
	    { OP_INT_SUBTRACT, keywords->subtract_int },
	    { OP_FLOAT_SUBTRACT, keywords->subtract_float },
	    
	    { OP_INT_MULTIPLY, keywords->multiply_int },
	    { OP_FLOAT_MULTIPLY, keywords->multiply_float },
	    
	    { OP_INT_DIVIDE, keywords->divide_int },
	    { OP_FLOAT_DIVIDE, keywords->divide_float },

	    {},
	};

    match = matches;

    while (match->name)
    {
	if (match->name == name)
	{
	    *result = match->opcode;
	    return true;
	}
	
	match++;
    }

    return false;
}
