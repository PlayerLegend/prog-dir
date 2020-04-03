#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stddef.h>
#endif

typedef struct list1_node list1_node;
typedef struct list1 list1;

struct list1_node {
    list1_node * next;
};

struct list1 {
    list1_node * begin;
    list1_node * end;
};

inline static void list1_add_last(list1 * list, list1_node * node)
{
    node->next = NULL;
    if(!list->end)
    {
	list->begin = list->end = node;
    }
    else
    {
	list->end->next = node;
	list->end = node;
    }
}

inline static void list1_add_next(list1 * list, list1_node * node)
{
    node->next = list->begin;
    list->begin = node;
    if(!list->end)
	list->end = node;
}

inline static list1_node * list1_pop(list1 * list)
{
    list1_node * ret = list->begin;
    list->begin = ret ? ret->next : NULL;
    if(list->begin == NULL)
	list->end = NULL;
    return ret;
}
