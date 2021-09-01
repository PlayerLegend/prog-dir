
inline static void setup_match (dzip_match * restrict match, dzip_window * restrict window, dzip_window_point point, const char * restrict input_begin, dzip_size input_size)
{
    match->point = point;

    match->length = 0;

    unsigned int i = 0;
    dzip_window_point i_point;
    
    while (i < input_size)
    {
	if (window->point == (i_point = (point + i) % count_array (window->begin)))
	{
	    break;
	}

	if (window->begin[i_point] != input_begin[i])
	{
	    break;
	}
	
	i++;
    }

    match->length = i;

    if (!match->length)
    {
	return;
    }

    while (i < input_size)
    {
	if (input_begin[i] != input_begin[i - match->length])
	{
	    break;
	}
	
	i++;
    }

    match->length = i;
}
