#include "array.h"
#include <stdbool.h>
#include "hash_table.h"

enum word_type {
    WORD_KEY,
    WORD_QUOTE,
    WORD_LIST,
};

typedef struct {
    int type;
    int content;
    int peer;
}
    lex_node;

typedef struct {
    table table;
    array(lex_node) nodes;
}
    lex_tree;

