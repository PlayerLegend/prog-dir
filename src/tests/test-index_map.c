#include "index_map.h"

int main()
{
    index_map(int) map = {0};

    const int max = 100;
    for(int i = 0; i < max; i++)
    {
	*index_map_access(&map,i) = i * 5;
    }

    for(int * i = map.begin; i < map.alloc; i++)
    {
	printf("%4zd: %d\n",i - map.begin, *i);
    }
}
