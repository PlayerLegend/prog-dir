#include <stdio.h>

#define FLAT_INCLUDES

#include "list1.h"

int main()
{
    int count = 10;
    list1 list = {};
    struct {
	list1_node node;
	int num;
    }
    nodes[count], *node;

    for(int i = 0; i < count; i++)
    {
	nodes[i].num = i;
	list1_add_last(&list,&nodes[i].node);
    }

    printf("forward\n");
    
    while( NULL != (node = (void*)list1_pop(&list)) )
    {
	printf("%d\n",node->num);
    }

    for(int i = 0; i < count; i++)
    {
	nodes[i].num = i;
	list1_add_next(&list,&nodes[i].node);
    }
    
    printf("\nreverse\n");
    
    while( NULL != (node = (void*)list1_pop(&list)) )
    {
	printf("%d\n",node->num);
    }

    return 0;
}
