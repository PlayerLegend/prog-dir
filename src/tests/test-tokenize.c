#include "tokenize.h"
#include <stdlib.h>

const char * identify(int type)
{
    static const char
	*unknown = "unknown",
	*generic = "generic",
	*quote = "quote",
	*special = "special";
    
    switch(type)
    {
    case TOKEN_GENERIC:
	return generic;
	
    case TOKEN_QUOTE:
	return quote;
	
    case TOKEN_SPECIAL:
	return special;
	
    default:
	return unknown;
    }
}

int main(int argc, char * argv[])
{
    if( argc != 2 )
	return 1;
    
    char * file_name = argv[1];
    
    FILE * file = fopen(file_name,"r");

    if(!file || ferror(file))
    {
	perror(__func__);
	return 1;
    }

    stack list = new_stack(token);
    
    tokenize_file(&list,file_name,file);

    for(token * i = list.begin; (void*)i < list.end; i++)
    {
	printf("%d,%d",i->line,i->col);
	printf(" '%s' (%s) [%s]\n",i->text,identify(i->type),i->file_name);
    }

    free(list.begin);

    return 0;
}
