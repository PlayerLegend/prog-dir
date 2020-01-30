#define FLAT_INCLUDES
#include <stdio.h>
#include "stack.h"
#include "array.h"
#include "print.h"
#include "print_array.h"
#include <string.h>
#include <stdlib.h>
#include "range.h"

int main()
{
    char * string1 = "asdf";
    char * string2 = "bcle";

    char * string_write = NULL;
    print_buffer_write(&string_write,"%s: %s",string1,string2);
    printf("write: '%s'\n",string_write);
    print_buffer_write(&string_write,"%s: %s",string2,string1);
    printf("write: '%s'\n",string_write);

    char * string_append = NULL;
    print_buffer_append(&string_append,"%s: ",string1);
    printf("append: '%s'\n",string_append);
    print_buffer_append(&string_append,"%s",string2);
    printf("append: '%s'\n",string_append);

    char_array array = { 0 };

    print_array_write(&array,"asdf writing the array once %d %d %s\n",100,200,"heres a string");
    printf("written array #1(%zu): '%.*s'\n",count_range(array),(int)count_range(array),array.begin);

    print_array_write(&array,"1234 writing the array again %s %d %s\n","some string #1",200,"some string #2");
    printf("written array #2(%zu): '%.*s'\n",count_range(array),(int)count_range(array),array.begin);

    print_array_append(&array,"appending the arrayyyyy %d %d %s\n",450,200,"heres a string again");
    printf("appended array(%zu): '%.*s'\n",count_range(array),(int)count_range(array),array.begin);
    
    return 0;
}
