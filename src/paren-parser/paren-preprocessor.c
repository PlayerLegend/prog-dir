#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../log/log.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../immutable/immutable.h"
#include "paren-parser.h"
#include "paren-preprocessor.h"

#define fatal(STR, node, ...) { log_error("%s:%d:%d: " STR, (node)->filename, (node)->line_number, (node)->column_number, ##__VA_ARGS__); goto fail; }

typedef enum include_status { INCLUDE_FAIL, INCLUDE_YES, INCLUDE_NONE } include_status;

typedef struct buffer(paren_atom*) buffer_atom_p;

typedef struct macro_arg_match macro_arg_match;
struct macro_arg_match { paren_atom_child * child; macro_arg_match * next; };

typedef struct macro_arg macro_arg;
struct macro_arg { const char * name; paren_atom * default_value; macro_arg_match * matches; macro_arg * next; };

typedef struct macro macro;
struct macro { const char * name; macro_arg * args; paren_atom * body; macro * next; };

typedef struct buffer(macro) buffer_macro;

void free_macro_arg_match (macro_arg_match * match)
{
    macro_arg_match * to_free;

    while (match)
    {
	to_free = match;
	match = match->next;

	to_free->child->is_text = false;
	to_free->child->atom = NULL;
	
	free (to_free);
    }
}

void free_macro_arg (macro_arg * arg)
{
    macro_arg * to_free;

    while (arg)
    {
	to_free = arg;
	arg = arg->next;

	free_macro_arg_match (to_free->matches);
	paren_atom_free (to_free->default_value);
	free (to_free);
    }
}

void free_macro (macro * _macro)
{
    macro * to_free;

    while (_macro)
    {
	to_free = _macro;
	_macro = _macro->next;

	free_macro_arg (to_free->args);
	paren_atom_free (to_free->body);
	free (to_free);
    }
}

static bool process_include (immutable_namespace * namespace, paren_atom ** parent)
{
    assert (*parent);
    assert ((*parent)->child.is_text == false);

    paren_atom * child = (*parent)->child.atom;

    assert (child->child.is_text == true);
    assert (child->child.text == immutable_string (namespace, "include"));

    paren_atom * new = NULL;
    paren_atom ** set = &new;

    while ( (child = child->peer) )
    {
	assert (*set == NULL);
	
	if (!child->child.is_text)
	{
	    fatal ("Including a non-text node is not supported", child);
	}

	*set = paren_parse (.namespace = namespace, .filename = immutable_path (namespace, child->child.text));
	
	if (!*set)
	{
	    goto fail;
	}

	while (*set)
	{
	    set = &(*set)->peer;
	}
    }

    assert (*set == NULL);

    *set = (*parent)->peer;
    (*parent)->peer = NULL;
    paren_atom_free(*parent);
    *parent = new;

    return true;

fail:
    return false;
}

paren_atom * _dupe_tree (paren_atom * root)
{
    paren_atom * retval = NULL;
    paren_atom * from = root;
    paren_atom ** to = &retval;

    while (from)
    {
	*to = calloc (1, sizeof (**to));
	**to = *from;

	if (!(*to)->child.is_text)
	{
	    (*to)->child.atom = _dupe_tree (from->child.atom);
	}

	from = from->peer;
	to = &(*to)->peer;
    }

    return retval;
}

static bool read_macro_arg_list (macro_arg ** set, paren_atom * arg_list)
{
    while (arg_list)
    {	
	*set = calloc (1, sizeof (**set));
	
	if (arg_list->child.is_text == false)
	{
	    paren_atom * arg_list_child_defval_key;

	    arg_list_child_defval_key = arg_list->child.atom;

	    if (arg_list_child_defval_key)
	    {
		if (!arg_list_child_defval_key->child.is_text)
		{
		    fatal ("Expected argument name", arg_list_child_defval_key);
		}

		(*set)->name = arg_list_child_defval_key->child.text;

		if (arg_list_child_defval_key->peer)
		{
		    if (arg_list_child_defval_key->peer->peer)
		    {
			fatal ("Multiple items specified as a default value for '%s'", arg_list_child_defval_key->peer->peer, arg_list_child_defval_key->child.text);
		    }
		    else
		    {
			(*set)->default_value = arg_list_child_defval_key->peer;
			arg_list_child_defval_key->peer = NULL;
		    }
		}
	    }
	}
        else
	{
	    (*set)->name = arg_list->child.text;
	}

	set = &(*set)->next;

	arg_list = arg_list->peer;
    }

    return true;
    
fail:
    return false;
}

static void add_arg_list_matches (macro_arg * list, paren_atom * root)
{
    struct buffer(paren_atom*) parent_peers = {0};

    paren_atom * child;

    macro_arg_match * new_match;
    
    while (list)
    {
	child = root;

	while (child || range_count (parent_peers))
	{
	    if (!child)
	    {
		child = *(--parent_peers.end);
	    }
	    
	    if (child->child.is_text)
	    {
		if (child->child.text == list->name)
		{
		    new_match = calloc (1, sizeof(*new_match));
		    new_match->next = list->matches;
		    list->matches = new_match;
		    new_match->child = &child->child;
		}

		child = child->peer;
	    }
	    else
	    {
		if (child->peer)
		{
		    *buffer_push (parent_peers) = child->peer;
		}
		
		child = child->child.atom;
	    }
	}
	
	list = list->next;
    }

    free (parent_peers.begin);
}

static macro * read_macro (immutable_namespace * namespace, paren_atom ** parent)
{
    macro * retval = calloc (1, sizeof (*retval));
    
    assert (*parent);
    assert ((*parent)->child.is_text == false);
    assert ((*parent)->child.atom);
    assert ((*parent)->child.atom->child.is_text == true);
    assert ((*parent)->child.atom->child.text == immutable_string (namespace, "macro"));

    struct macro_tree { paren_atom *keyword, *name, *arg_list; } macro_tree;

    macro_tree.keyword = (*parent)->child.atom;

    if (!macro_tree.keyword)
    {
	fatal ("malformed macro: missing 'macro' keyword", *parent);
    }

    macro_tree.name = macro_tree.keyword->peer;

    if (!macro_tree.name)
    {
	fatal ("malformed macro: missing name", macro_tree.keyword);
    }

    if (!macro_tree.name->child.is_text)
    {
	fatal ("malformed macro: expected text name", macro_tree.name);
    }

    macro_tree.arg_list = macro_tree.name->peer;

    if (!macro_tree.arg_list)
    {
	fatal ("malformed macro: missing argument list", macro_tree.name);
    }

    if (macro_tree.arg_list->child.is_text)
    {
	fatal ("expected argument list, got text", macro_tree.arg_list);
    }

    retval->body = macro_tree.arg_list->peer;
    macro_tree.arg_list->peer = NULL;

    if (!retval->body)
    {
	fatal ("malformed macro: missing body", macro_tree.arg_list);
    }

    retval->name = macro_tree.name->child.text;
    
    if (!read_macro_arg_list(&retval->args, macro_tree.arg_list->child.atom))
    {
	goto fail;
    }
    
    paren_atom * to_free = *parent;
    *parent = (*parent)->peer;
    to_free->peer = NULL;
    paren_atom_free(to_free);

    add_arg_list_matches (retval->args, retval->body);

    return retval;
    
fail:
    return NULL;
}

static bool apply_macro (immutable_namespace * namespace, paren_atom ** parent, macro * macro)
{
    assert ( !(*parent)->child.is_text );
    assert ( (*parent)->child.atom );
    assert ( (*parent)->child.atom->child.is_text );
    assert ( (*parent)->child.atom->child.text == macro->name );
    
    paren_atom * input_arg = (*parent)->child.atom->peer;
    macro_arg * output_arg = macro->args;

    while (input_arg && output_arg)
    {
	for (macro_arg_match * match = output_arg->matches; match; match = match->next)
	{
	    *match->child = input_arg->child;
	}

	input_arg = input_arg->peer;
	output_arg = output_arg->next;
    }

    if (output_arg)
    {
	fatal ("Too few arguments to macro '%s'", *parent,  macro->name);
    }

    if (input_arg)
    {
	fatal ("Too many arguments to macro '%s'", *parent,  macro->name);
    }

    paren_atom * to_free = *parent;
    paren_atom * to_insert = _dupe_tree(macro->body);

    *parent = to_insert;
    while (to_insert->peer)
    {
	to_insert = to_insert->peer;
    }
    to_insert->peer = to_free->peer;
    to_free->peer = NULL;

    paren_atom_free(to_free);

    return true;
fail:
    return false;
}

paren_atom * _paren_preprocessor (struct paren_parse_arg arg)
{
    const char * include_string = immutable_string (arg.namespace, "include");
    const char * macro_string = immutable_string (arg.namespace, "macro");

    paren_atom * retval = _paren_parse (arg);
    
    typedef struct layer layer;
    struct layer { paren_atom ** set; macro * macros; layer * next; };
    
    layer * layers = calloc (1, sizeof(*layers));

    paren_atom * child;

    layers->set = &retval;

    while (layers)
    {
	assert (layers);
	assert (layers->set);
	
	if (!*layers->set)
	{
	    goto parent_peer;
	}

	if ((*layers->set)->child.is_text)
	{
	    goto peer;
	}
	else
	{
	    if ((child = (*layers->set)->child.atom) && child->child.is_text)
	    {
		if (child->child.text == include_string)
		{
		    if (!process_include (arg.namespace, layers->set))
		    {
			goto fail;
		    }
		    
		    continue;
		}
		else if (child->child.text == macro_string)
		{
		    macro * new_macro = read_macro (arg.namespace, layers->set);
		    
		    if (!new_macro)
		    {
			goto fail;
		    }
		    
		    new_macro->next = layers->macros;
		    layers->macros = new_macro;
		    
		    continue;
		}
		else
		{
		    for (layer * check_layer = layers; check_layer; check_layer = check_layer->next)
		    {
			for (macro * check_macro = check_layer->macros; check_macro; check_macro = check_macro->next)
			{
			    if (check_macro->name == child->child.text)
			    {
				if (apply_macro (arg.namespace, layers->set, check_macro))
				{
				    goto restart;
				}
				else
				{
				    goto fail;
				}
			    }
			}
		    }

		    
		}
	    }

	    goto child;
	}

	assert (false);

    parent_peer:
	{
	    layer * to_free = layers;
	    layers = layers->next;

	    free_macro (to_free->macros);
	    free (to_free);

	    if (layers && *layers->set)
	    {
		layers->set = &(*layers->set)->peer;
	    }
	    
	    continue;
	}

    peer:
	{
	    layers->set = &(*layers->set)->peer;
	    continue;
	}

    child:
	{
	    layer * new = calloc (1, sizeof (*new));
	    new->set = &(*layers->set)->child.atom;
	    new->next = layers;
	    layers = new;
	    continue;
	}

    restart:
	continue;
    }
    
    return retval;

fail:
    return NULL;
};
