#include "tokenize.h"

#include "serial.h"

#include <stdlib.h>

#include <stdbool.h>

bool
in_list(char c, char * list)
{
    while(*list)
    {
	if(c == *list)
	    return true;
	list++;
    }

    return false;
}

void _add_to_list(token_list * build_list, stack * build_token, int line, int col, bool force_empty,int type,const char * file_name)
{
    if(build_token->begin == build_token->end && !force_empty)
	return;

    *(char*)stack_push(build_token) = '\0';
    token * set = stack_type_push(build_list,token);
    *set = (token)
	{
	    .type = type,
	    .file_name = file_name,
	    .line = line,
	    .col = col,
	    .text = build_token->begin,
	};
//    printf("add %d,%d %s\n",set->line,set->col,set->text);
    stack_reset(build_token);
}

void tokenize_file(token_list * list, const char * file_name, FILE * file)
{
    serial read;

    serial_file(&read,file);

    char c;

    char current_quote = '\0';

    char space[] = " \n\t";

    char special[] = "()";

    char quote[] = "\"\'";

    stack build_token = new_stack(char);

    bool update_start = false;
    bool newline = true;

    struct
    {
	int line, col;
    }
    point = { -1, 0 }, token_start = { 0, 0 };

    while( (c = serial_read(read)) )
    {
	if(newline)
	{
	    point.line++;
	    point.col = 0;
	    newline = false;
	}
	else
	{
	    point.col++;
	}
	
	if( update_start )
	{
	    token_start = point;
	    update_start = false;
	}

	if(c == '\n')
	    newline = true;

	if(current_quote)
	{
	    if(c == current_quote)
	    {
		_add_to_list(list,&build_token,token_start.line,token_start.col,true,TOKEN_QUOTE,file_name);
		update_start = true;
		current_quote = '\0';
	    }
	    else
	    {
		*(char*)stack_push(&build_token) = c;
	    }
	    
	    continue;
	}
	
	if( in_list(c,quote) )
	{
	    current_quote = c;
	    continue;
	}

	if( in_list(c,special) )
	{
	    _add_to_list(list,&build_token,token_start.line,token_start.col,false,TOKEN_GENERIC,file_name);
	    update_start = true;
	    
	    char * special = malloc(2);
	    special[0] = c;
	    special[1] = '\0';
	    *stack_type_push(list,token) = (token)
	    {
		.type = TOKEN_SPECIAL,
		.file_name = file_name,
		.line = point.line,
		.col = point.col,
		.text = special,
	    };
	    continue;
	}

	if( in_list(c,space) )
	{
	    _add_to_list(list,&build_token,token_start.line,token_start.col,false,TOKEN_GENERIC,file_name);
	    update_start = true;
	    continue;
	}
	
	*(char*)stack_push(&build_token) = c;
    }
    
    _add_to_list(list,&build_token,token_start.line,token_start.col,false,current_quote ? TOKEN_QUOTE : TOKEN_GENERIC,file_name);
}

void build_token_tree(tree * tree, token_list * list)
{
    tree->stack = new_stack(tree_node);

    tree_node * new;
    void ** last = NULL;

    stack parent_stack = new_stack(tree_node);
    
    for(token * i = list->begin; i != list->end; i++)
    {
	if( *i->text == '(' )
	{
	    last = &new->child;
	}
	else if( *i->text == ')' )
	{
	    
	}
	else
	{
	    new = tree_add(tree);

	    *new = (tree_node){0};
	    
	    if(last)
		*last = new;

	    last = &new->peer;
	}
    }
}
