#include "precompiled.h"

#define FLAT_INCLUDES
//#include "range.h"
#include "line_parse.h"

void clean_trailing(char * string, char target)
{
    char * end = strchr(string,target);
    if(end)
	*end = '\0';
}

void delimit(delimited_string * out, char * input, char delim)
{
    array_rewrite(out);
    
    *array_push(out) = input;

    char * next;

    while( (next = strchr(next,delim)) )
    {
	*next = '\0';
	next++;
	*array_push(out) = next;
    }
}

void print_delimited(delimited_string * print)
{
    for_range(word,*print)
	printf("[%s] ",*word);
    printf("\n");
}
