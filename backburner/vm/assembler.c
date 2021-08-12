#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../immutable/immutable.h"
#include "../paren-parser/paren-parser.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../log/log.h"
#include "arch.h"
#include "machine-code.h"
#include "assembler.h"
#include "text.h"
#include "constants.h"

#define fatal(STR, node, ...) { log_error("%s:%d:%d: " STR, (node)->filename, (node)->line_number, (node)->column_number, ##__VA_ARGS__); goto fail; }

typedef struct name name;
struct name
{
    const char * key;
    vm_address value;
    bool is_constant;
    const char * type;
};

typedef struct buffer(name) buffer_name;

typedef struct scope scope;
struct scope
{
    buffer_name namees;
    paren_atom * node;
    scope * next;
};

typedef struct compile_state compile_state;
struct compile_state
{
    machine_code * machine_code;
    keywords * keywords;
};

name * resolve_name_local (buffer_name * names, const char * key)
{
    name * match;
    for_range (match, *names)
    {
	if (match->key == key)
	{
	    return match;
	}
    }

    return NULL;
}

name * resolve_name (scope * scope, const char * key)
{
    name * match = NULL;

    while ( scope && !(match = resolve_name_local (&scope->namees, key)) )
    {
	scope = scope->next;
    }

    return match;
}

#define add_name(names, ...) _add_name (names, (name){__VA_ARGS__})
bool _add_name (buffer_name * names, name add)
{
    if (resolve_name_local (names, add.key))
    {
	return false;
    }
    
    *buffer_push (*names) = add;

    return true;
}

bool resolve_constant (vm_address * result, compile_state * state, scope * parent_scope, paren_atom * root)
{
    
}

bool add_constant (buffer_char * constants, keywords * keywords, scope * scope, paren_atom * instruction_name)
{
    assert (instruction_name->child.text == keywords->name);
    
    paren_atom * name_atom = instruction_name->peer;
    
    if (!name_atom || !name_atom->child.is_text)
    {
	fatal ("Expected a text name in constant definiton'", instruction_name);
    }

    paren_atom * value_atom = name_atom->peer;

    vm_address value_address;

    name new_name;

    if (!resolve_constant (&value_address, constants, keywords, scope, value_atom))
    {
	goto fail;
    }

    if (!add_name (&scope->namees, .key = name_atom->child.text, .value = value_address))
    {
	goto fail;
    }

    return true;
    
fail:
    return false;
}

void free_scope (scope * scope)
{
    
}

static bool parse_sum (machine_code * result, scope * parent_scope, keywords * keywords, paren_atom * root)
{
    enum sum_type { SUMTYPE_INT, SUMTYPE_FLOAT };
    struct sum_item { enum sum_type type; union { vm_int _int; vm_float _float; }; };

    struct buffer (struct sum_item) items = {0};

    assert (root->child.is_text);
    assert (root->child.text == keywords->sum);

    paren_atom * item_atom = root;

    while ( (item_atom = item_atom->peer) )
    {
	if (!item_atom->child.is_text)
	{
	    
	}
    }
}

static bool parse_recurse (machine_code * result, scope * parent_scope, keywords * keywords, paren_atom * root);
static bool parse_ifelse (machine_code * result, scope * parent_scope, keywords * keywords, paren_atom * root)
{
    paren_atom * ifatom = root->peer;

    if (!ifatom)
    {
	fatal ("%s requires a body", root, root->child.text);
    }

    if (ifatom->child.is_text)
    {
	fatal ("%s body must be a routine", ifatom, root->child.text);
    }
    
    paren_atom * elseatom = ifatom->peer;
    
    vm_address ifjump_location = range_count (result->instructions);
    buffer_push (result->instructions);
    
    if (!parse_recurse (result, parent_scope, keywords, ifatom->child.atom))
    {
	goto fail;
    }

    if (elseatom)
    {
	if (elseatom->child.is_text)
	{
	    fatal ("%s else body must be a routine", elseatom, root->child.text);
	}
	
	vm_address elsejump_location = range_count (result->instructions);
	buffer_push (result->instructions);

	log_debug ("elsejump: %d", OP_JUMP);
	
	if (!parse_recurse (result, parent_scope, keywords, elseatom->child.atom))
	{
	    goto fail;
	}

	result->instructions.begin[elsejump_location] = (instruction) { .opcode = OP_JUMP, .arg = range_count (result->instructions) };
	result->instructions.begin[ifjump_location] = (instruction) { .opcode = OP_JUMP_IF_NOT, .arg = elsejump_location + 1 };
    }
    else
    {
	result->instructions.begin[ifjump_location] = (instruction){ .opcode = OP_JUMP_IF_NOT, .arg = range_count (result->instructions) };
    }

    return true;
    
fail:
    return false;
}

static bool parse_instruction (machine_code * result, scope * parent_scope, keywords * keywords, paren_atom * root)
{
    assert (root);
    assert (root->child.is_text);
    
    instruction new_instruction = {0};
    
    if (!identify_string_opcode (&new_instruction.opcode, keywords, root->child.text))
    {
	fatal ("Invalid opcode '%s'", root, root->child.text);
    }

    paren_atom * arg = root->peer;

    switch (new_instruction.opcode)
    {
    default:
	if (!arg)
	{
	    fatal ("Expected argument to '%s'", root, root->child.text);
	}

	if (!resolve_constant (&new_instruction.arg, &result->constants, keywords, parent_scope, arg))
	{
	    goto fail;
	}
    
	break;

    case OP_EXIT:
	if (arg)
	{
	    fatal ("Expected no argument for '%s'", arg, root->child.text);
	}

	new_instruction.arg = 0;
	break;
    }

    *buffer_push (result->instructions) = new_instruction;

    return true;

fail:
    return false;
}

static bool parse_recurse (machine_code * result, scope * parent_scope, keywords * keywords, paren_atom * root)
{
    assert (!root->child.is_text);
    assert (root->child.atom);
    
    paren_atom * child = root->child.atom;

    while (root)
    {
	child = root->child.atom;

	if (!child->child.is_text)
	{
	    scope new_scope = { .next = parent_scope };
	    if (!parse_recurse (result, &new_scope, keywords, child))
	    {
		goto fail;
	    }
	}
	else
	{
	    const char * function_name = child->child.text;
	
	    if (function_name == keywords->ifelse)
	    {
		if (!parse_ifelse (result, parent_scope, keywords, child))
		{
		    goto fail;
		}
	    }
	    else if (function_name == keywords->name)
	    {
		if (!add_constant (&result->constants, keywords, parent_scope, child))
		{
		    goto fail;
		}
	    }
	    else
	    {
		if (!parse_instruction (result, parent_scope, keywords, child))
		{
		    goto fail;
		}
	    }
	}

	root = root->peer;
    }

    return true;
    
fail:
    return false;
}

bool assembler (machine_code * result, table_string * table, paren_atom * root)
{
    keywords keywords = {0};

    init_keywords (&keywords, table);

    scope scope = {0};
    
    return parse_recurse (result, &scope, &keywords, root);
}
