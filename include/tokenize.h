#ifndef FLAT_INCLUDES
#include <stdio.h>
#define FLAT_INCLUDES
#include "range.h"
#include "tree.h"
#endif

typedef struct
{
    enum { TOKEN_GENERIC, TOKEN_QUOTE, TOKEN_SPECIAL } type;
    const char * file_name;
    int line;
    int col;
    char * text;
}
    token;

typedef stack_type(token) token_list;

void tokenize_file(token_list * list, const char * file_name, FILE * file);

void build_token_tree(tree * tree, token_list * list);
