#ifndef FLAT_INCLUDES
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#endif

typedef struct {
    unsigned char * begin;
    size_t size;
    size_t eldest;
}
    sliding_window;

inline static void sliding_window_init(sliding_window * window, size_t size)
{
    window->size = size;
    window->eldest = 0;
    window->begin = calloc(size, sizeof(*window->begin));
}

inline static void sliding_window_push(sliding_window * window, unsigned char c)
{
    window->begin[window->eldest] = c;
    window->eldest = (window->eldest + 1) % window->size;
}

inline static size_t sliding_window_match_length (sliding_window * window, size_t index, const range_const_unsigned_char * match)
{
    size_t start = (window->eldest + index) % window->size;
    size_t size = window->size - start;
    size_t match_size = range_count(*match);
    if (size > match_size)
    {
	size = match_size;
    }

    size_t i;

    for (i = 0; i < size; i++)
    {
	//printf("check [%zu] '%d' == [%zu] '%d'\n", i, match->begin[i], start + i, window->begin[start + i]);
	
	if (match->begin[i] != window->begin[start + i])
	{
	    return i;
	}
    }

    //return i;

    size_t first_length = size;

    size = start;

    if (size > match_size - first_length)
    {
	size = match_size - first_length;
    }

    for (i = 0; i < size; i++)
    {
	//printf("check2 [%zu] '%d' == [%zu] '%d'\n", first_length + i, match->begin[first_length + i], i, window->begin[i]);
	if (match->begin[first_length + i] != window->begin[i])
	{
	    break;
	}
    }

    return first_length + i;
}
