#include "hash_table.h"
#include "hash_table_string.h"

typedef struct {
    unsigned int peer, child, string, type;
}
    lex_word;

typedef struct {
    unsigned int base, child, quote;
}
    lex_parser_types;

typedef struct {
    lex_word * root;
    table strings;
}
    lex_tree;

