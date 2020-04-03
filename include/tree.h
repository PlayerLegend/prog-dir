#ifndef FLAT_INCLUDES
#include <stdio.h>
#define FLAT_INCLUDES
#include "range.h"
#endif

typedef struct
{
    void * peer;
    void * child;
}
    tree_node;

typedef array(tree_node) tree;

tree_node * tree_add(tree * add);

#define is_tree_node(treep,ptr)			\
    ((ptr) >= (treep)->begin && (ptr) < (treep)->end)
