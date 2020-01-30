#include "delimit.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "range.h"

static char find_c(char c, char * list)
{
    if(!list)
	return '\0';
    
    char test;
    while( (test = *list++) )
	if(test == c)
	    break;

    return test;
}

static char * dupe_str(const char * str)
{
    return strcpy(malloc(strlen(str) + 1),str);
}

#define finalize_word(out,build)		\
    if(!is_range_empty(build))			\
    {  	*array_push(&build) = '\0';		\
	fflush(stdout);				\
	*array_push(out) = (build).begin;	\
	array_forget(&(build)); }

#define add_char(build,c)			\
    (*array_push(&(build)) = c)

int delimit_list(delimited_list * out, const delimit_config * config, const char * input)
{
    char c;
    char quote = '\0';
    array(char) build = { 0 };
    bool escape = false;

    while( (c = *input++) )
    {
	if(escape)
	{
	    add_char(build,c);
	    escape = false;
	    continue;
	}
	
	if(c == config->escape)
	{
	    escape = true;
	    continue;
	}
	
	if(quote)
	{
	    if(c == quote)
	    {
		quote = '\0';
		finalize_word(out, build);
	    }
	    else
	    {
		add_char(build,c);
	    }

	    continue;
	}

	quote = find_c(c,config->quote);
	if(quote)
	    continue;

	if(find_c(c,config->whitespace))
	{
	    finalize_word(out, build);
	    continue;
	}

	add_char(build,c);
    }

    finalize_word(out,build);

    return 0;
}

int count_c(char * start, char c)
{
    int count = 0;
    while(*start++ == c)
	count++;
    
    return count;
}

int delimit_clause(clause * out, const clause_config * config, const char * input)
{
    assert(*config->separator_list != '\0');

    const char * end_subject;
    const char * begin_predicate;

    end_subject = input;
    
    while(*input && !find_c(*end_subject,config->separator_list))
	end_subject++;

    if(!*input)
    {
	printf("Could not find '%s' in '%s'\n",config->separator_list,input);
	return -1;
    }

    begin_predicate = end_subject;
    
    while(*input && find_c(*begin_predicate,config->separator_list))
	begin_predicate++;


    int sep_count = begin_predicate - end_subject;
    
    if(config->separator_count)
    {
	if(sep_count < config->separator_count)
	{
	    *out = (clause){ 0 };
	    return -1;
	}
	else
	{
	    out->predicate = dupe_str(end_subject + config->separator_count);
	    out->subject = strndup(input,end_subject - input);
	    return 0;
	}
    }
    else
    {
	out->predicate = dupe_str(begin_predicate);
	out->subject = strndup(input,end_subject - input);
	return 0;
    }
}

void delimit_terminate(char * text, char end)
{
    char * find = strchr(text,end);
    if(find)
	*find = '\0';
}

void clear_clause(void * target)
{
    free( ((clause*)target)->subject );
    free( ((clause*)target)->predicate );
    *(clause*)target = (clause){ 0 };
}

void clear_delimited_list(void * target)
{
    delimited_list * cast = target;

    for_range(word,*cast)
    {
	free(*word);
    }

    free(cast->begin);

    *cast = (delimited_list){ 0 };
}
