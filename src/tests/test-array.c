#include "array.h"
#include "for_range.h"
#include <assert.h>

int main()
{
    typedef short test_int;
    array(test_int) test = {0};

    const test_int max = 100;
    for(test_int i = 0; i < max; i++)
    {
	printf("add [%d]\n",i);
	*array_push(&test) = -i;
    }

    //for(test_int * i = test.begin; i < test.end; i++)
    for_range(i,test)
    {
	printf("iterate [%zd]: %d\n",i - test.begin,*i);
    }

    for(test_int i = 0; i < max; i++)
    {
	printf("pop [%d]: %d\n",i,*array_pop(&test));
    }

    assert(test.begin == test.end);

    test_int ints[5] = { 2, 4, 6, 8, 10 };

    array_append_several(&test,ints,5);

    printf("added %d, %d, %d, %d, %d\n",ints[0],ints[1],ints[2],ints[3],ints[4]);

    while(test.begin != test.end)
    {
	printf("pop [%zd]: %d\n",test.end - test.begin,*array_pop(&test));
    }

    printf("\nadding 100 and deleting the first 50\n");
    
    for(test_int i = 0; i < max; i++)
    {
	*array_push(&test) = -i;
    }

    array_delete_first(&test,50);

    while(test.begin != test.end)
    {
	printf("pop [%zd]: %d\n",test.end - test.begin,*array_pop(&test));
    }
    
    return 0;
}
