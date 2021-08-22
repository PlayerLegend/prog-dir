
    /*
    if (recent_byte && recent_byte < max_ofs)
    {
	for_range (i_input, scan)
	{
	    if (*i_input != *(i_input - recent_byte))
	    {
		break;
	    }
	}
	
	test_length = range_index(i_input, scan);
    }
    */
    /*size_t start = recent_byte;

    if (recent_match && recent_match < max_ofs)
    {
	start = recent_match;
    }
    
    //log_debug ("start: %zu", start);

    assert (start > 0);*/
    
    /*for (i_ofs = start; i_ofs < max_ofs; i_ofs++)
    {
	for_range (i_input, scan)
	{
	    if (*i_input != *(i_input - i_ofs))
	    {
		break;
	    }
	}

	test_length = range_index(i_input, scan);

	if (test_length > match->length)
	{
	    match->length = test_length;
	    match->distance = i_ofs;
	    match->next_char = input->begin[test_length];
	}
	break;
	}*/

    //return match->length;
