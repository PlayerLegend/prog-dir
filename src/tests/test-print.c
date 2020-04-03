#include "precompiled.h"

#define FLAT_INCLUDES

//#include "range.h"
//#include "print.h"

int main()
{
    char_array array = {};

    printf("===============================================================================\n");
    for(int i = 0; i < 20; i += 2)
    {
	int n = print_array_write(&array,"Line %d\n",i);
	printf("Written array %d (%d == %zd): %s",i,n,array.end - array.begin,array.begin);
	assert( (ssize_t)n == (array.end - array.begin));
	assert(array.end[0] == '\0');
	assert(array.end[-1] != '\0');
    }

    printf("===============================================================================\n");
    free(array.begin);
    array_forget(&array);
    print_array_write(&array,"Initial\n");
    
    for(int i = 0; i < 10; i++)
    {
	ssize_t initial = array.end - array.begin;
	int n = print_array_append(&array,"Line %d\n",i);
	printf("Appended initialized array %d (%d == %zd): %s",i,n,(array.end - array.begin) - initial,array.begin);
	assert( (ssize_t)n == (array.end - array.begin) - initial);
	assert(array.end[0] == '\0');
	assert(array.end[-1] != '\0');
    }

    printf("===============================================================================\n");
    free(array.begin);
    array_forget(&array);
    
    for(int i = 0; i < 10; i++)
    {
	ssize_t initial = array.end - array.begin;
	int n = print_array_append(&array,"Line %d\n",i);
	printf("Appended non-initialized array %d (%d == %zd): %s",i,n,(array.end - array.begin) - initial,array.begin);
	assert( (ssize_t)n == (array.end - array.begin) - initial);
	assert(array.end[0] == '\0');
	assert(array.end[-1] != '\0');
    }
    printf("===============================================================================\n");
    printf("done\n");
    free(array.begin);
    array_forget(&array);
    /*char * string1 = "asdf";
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

    free(array.begin);
    array = (char_array){0};

    print_array_append(&array,"appending an empty array %d %d %s\n",451,201,"heres a string again1");
    printf("initial appended array(%zu): '%.*s'\n",count_range(array),(int)count_range(array),array.begin);
    print_array_append(&array,"appending again the empty array %d %d %s\n",452,202,"heres a string again2");
    printf("again appended array(%zu): '%.*s'\n",count_range(array),(int)count_range(array),array.begin);
    log_normal("Normal log %d %d %d",1,2,3);

    log_error("print some error %s",", some string");
    log_debug("print some debug %s","another some string");

    log_error("Logging error %d %d %d",3,2,1);
    
    printf("asdf\n");
    */
    return 0;
}
