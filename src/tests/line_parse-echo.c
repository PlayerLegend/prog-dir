#include "line_parse.h"
#include <stdlib.h>

int main()
{
    char * line;
    size_t len;

    delimited_string delimited = {0};

    while( -1 != getline(&line,&len,stdin) )
    {
	clean_trailing(line,'\n');
	delimit(&delimited,line,' ');
	print_delimited(&delimited);
    }

    free(line);
}
