#include "stack.h"

#include <stdio.h>

int main()
{
    const int count = 100;

    printf("pushing %d ints\n",count);

    stack push = { 0 };

    for(int i = 0; i < count; i++)
    {
	printf("push %d\n",i);
	*stack_push_type(&push,int) = i;
    }

    printf("After pushing, there are %zd elements\n", ((char*)push.end - (char*)push.begin) / sizeof(int));
    
    for(int * i = push.begin; (void*)i < push.end; i++)
    {
	printf("i: %d\n",*i);
    }

    stack_free(&push);

    return 0;
}
