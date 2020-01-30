#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include "stack.h"
#endif

typedef struct
{
    void * peer;
    void * child;
}
    tree_node;

typedef stack_type(tree_node) tree;

tree_node * tree_add(tree * add);

#define is_tree_node(treep,ptr)			\
    ((ptr) >= (treep)->begin && (ptr) < (treep)->end)
