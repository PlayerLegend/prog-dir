#include "print_array.h"
#include "range.h"
#include <stdarg.h>

#include <string.h>
#include <assert.h>

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
    new = vsnprintf(NULL,0,fmt,va);
    va_end(va);

    size_t have = count_range(*array);
    size_t need = have + new;

    assert(array->end == NULL || *array->end == '\0');

    printf("have: %zu, need %zu\n",have,need);

    array_alloc(array,need + 1);

    array->end = array->begin + need;
    
    va_start(va,fmt);
    vsnprintf(array->end - new,new + 1,fmt,va);
    va_end(va);

    *array->end = '\0';
    
    assert(array->end == NULL || *array->end == '\0');
    
    return new - 1;  
}
