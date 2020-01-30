#include "print_array.h"
#include "range.h"
#include <stdarg.h>

int print_array_write(char_array * array, const char * fmt, ...)
{
    va_list va;

    size_t need;

    va_start(va,fmt);
    need = vsnprintf(NULL,0,fmt,va) + 1;
    va_end(va);

    array_alloc(array,need);
    array_rewrite(array);
    
    va_start(va,fmt);
    vsnprintf(array->begin,need,fmt,va);
    va_end(va);

    array->end = array->begin + need;
    
    return need - 1;
}

int print_array_append(char_array * array, const char * fmt, ...)
{
    va_list va;

    size_t new;

    va_start(va,fmt);
    new = vsnprintf(NULL,0,fmt,va) + 1;
    va_end(va);

    size_t have = count_range(*array);
    size_t need = have + new;

    if(!is_range_empty(*array))
	have--;

    printf("have: %zu, need %zu\n",have,need);

    array_alloc(array,need);
    
    va_start(va,fmt);
    vsnprintf(array->begin + have,new,fmt,va);
    va_end(va);
    array->end = array->begin + need;
    
    return new - 1;  
}
