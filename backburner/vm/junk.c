
static bool resolve_constant (vm_address * result, buffer_char * constants, keywords * keywords, scope * scope, paren_atom * root)
{	
    union { vm_address address; vm_float floatval; vm_int intval; name * name; } constant;

    if (root->child.is_text)
    {
	log_debug ("resolving text node '%s'", root->child.text);

	if (read_vm_address (&constant.address, root->child.text))
	{
	    *result = constant.address;
	}
	if (read_vm_int (&constant.intval, root->child.text))
	{
	    *result = use_constant (constants, &constant.intval, sizeof (constant.intval));
	}
	else if (read_vm_float (&constant.floatval, root->child.text))
	{
	    *result = use_constant (constants, &constant.floatval, sizeof (constant.floatval));
	}
	else if ( (constant.name = resolve_name (scope, root->child.text)) )
	{
	    *result = constant.name->value;
	}
	else
	{
	    fatal ("Unrecognized operand '%s'", root, root->child.text);
	}

	return true;
    }
    else
    {
	paren_atom * type = root->child.atom;
	
	if (!type)
	{
	    fatal ("Expected constant definition, got empty list", root);
	}

	if (!type->child.is_text)
	{
	    fatal ("Expected type name", type);
	}

	paren_atom * value = type->peer;

	if (!value)
	{
	    fatal ("Expected constant definiton following type", type);
	}

	if (!value->child.is_text)
	{
	    fatal ("Expected text constant definition following type", value);
	}

	if (type->child.text == keywords->_int)
	{
	    if (!read_vm_int (&constant.intval, value->child.text))
	    {
		fatal ("Failed to parse integer '%s'", value, value->child.text);
	    }
	    else
	    {
		*result = use_constant (constants, &constant.intval, sizeof (constant.intval));
		return true;
	    }
	}
	else if (type->child.text == keywords->_float)
	{
	    if (!read_vm_float (&constant.floatval, value->child.text))
	    {
		fatal ("Failed to parse float '%s'", value, value->child.text);
	    }
	    else
	    {
		*result = use_constant (constants, &constant.floatval, sizeof (constant.floatval));
		return true;
	    }
	}
	else
	{
	    fatal ("Invalid type name '%s'", type, type->child.text);
	}
	
	assert (false);
    }

    assert (false);

fail:
    return false;
}
