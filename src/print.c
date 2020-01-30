#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

int
_print_error(const char * file, const char * func, const char * fmt, ...)
{
    va_list va;

    int ret = 0;

    va_start(va,fmt);

    if( 0 > pthread_mutex_lock(&print_mutex) )
	return -1;
    
    ret += fprintf(stderr,"error: %s: %s: ",file,func);
    ret += vfprintf(stderr,fmt,va);
    ret += fprintf(stderr,"\n");
  
    if( 0 > pthread_mutex_unlock(&print_mutex) )
    {
	va_end(va);
	return -1;
    }
    
    va_end(va);

    fflush(stderr);
  
    return ret;
}

int print_buffer_write(char ** string, const char * fmt, ...)
{
    va_list va;

    int ret = 0;

    va_start(va,fmt);
    ret = vsnprintf(NULL,0,fmt,va);
    va_end(va);

    if(NULL == *string)
	*string = malloc(ret + 1);
    else
	*string = realloc(*string,ret + 1);
    
    **string = '\0';
    va_start(va,fmt);
    vsnprintf(*string,ret + 1,fmt,va);
    va_end(va);
    
    return ret;
}

int print_buffer_append(char ** string, const char * fmt, ...)
{
    va_list va;

    int ret = 0;

    va_start(va,fmt);
    ret = vsnprintf(NULL,0,fmt,va);
    va_end(va);

    size_t have;

    if(NULL == *string)
    {
	have = 0;
	*string = malloc(ret + 1);
    }
    else
    {
	have = strlen(*string);
	*string = realloc(*string,have + ret + 1);
    }
    
    va_start(va,fmt);
    vsnprintf(have + *string,ret + 1,fmt,va);
    va_end(va);
    
    return ret;
}
