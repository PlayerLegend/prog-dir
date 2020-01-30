#include "read_config_lines.h"
#include <assert.h>
#include <stdlib.h>

#define WHITESPACE " \t"
#define LINE_COMMENT '#'

static void terminate(char * string)
{
    char * end = strchr(string,'\n');
    if(end)
	*end = '\0';
}

static bool char_match(char c, char * list)
{
    char check;

    while( (check = *list++) )
    {
	if(!check)
	    return false;
	
	if(c == check)
	    return true;
    }

    assert(false);
    return false;
}

char * find_word(char * line)
{
    while(*line && char_match(*line,WHITESPACE))
	line++;
}

int text_config_read_file(text_config * config, FILE * file)
{
    struct { char * text; size_t len; } line = { 0 };
    char * parse;
    text_config_line * build_line;
    
    while( -1 != getline(&line.text,&line.len,file) )
    {
	terminate(line.text);
	
	parse = line.text;

	parse = find_word(parse);

	if(!*parse || *parse == LINE_COMMENT)
	    continue;

	do {
	    build_line = array_push(config);
	    *build_line = (text_config_line){ 0 };

	    
	} while(*parse);
    }

    free(line.text);
}
