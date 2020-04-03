#include "precompiled.h"

#define FLAT_INCLUDES

//#include "range.h"
//#include "print.h"

//static pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int logs;
    char * path;
    FILE * file;
}
    log_file;

static array(log_file) log_files;
bool logs_initialized;

void cleanup_logs()
{
    for_range(log,log_files)
    {
	fclose(log->file);
	free(log->path);
    }
    
    free(log_files.begin);
}

int register_log_path(int logs, const char * path)
{   
    if(!logs_initialized)
    {
	atexit(cleanup_logs);
	logs_initialized = true;
    }

    for_range(log,log_files)
    {
	if( 0 == strcmp(log->path,path) )
	{
	    log->logs |= logs;
	    return 0;
	}
    }

    FILE * file;

    if( NULL == (file = fopen(path,"a")) )
    {
	perror(path);
	log_error("Failed to open log file %s", path);
	return -1;
    }

    log_file * log = array_push(&log_files);
    log->logs = logs;
    log->file = file;
    log->path = strcpy(malloc(strlen(path) + 1),path);

    return 0;
}

int vprint_log(int logs, const char * fmt, va_list va)
{
    va_list va2;

    int ret = 0;

    
    for_range(log,log_files)
    {
	if(log->logs & logs)
	{
	    va_copy(va2,va);
	    ret = vfprintf(log->file,fmt,va);
	    fputc('\n',log->file);
	    va_end(va2);
	}
    }

    if(logs & (LOG_ERROR | LOG_DEBUG))
    {
	ret = vfprintf(stderr,fmt,va);
	fflush(stderr);
    }
    else if(logs & LOG_NORMAL)
    {
	ret = vprintf(fmt,va);
    }
    return ret;
}

int print_log(int logs, const char * fmt, ...)
{
    va_list va;
    va_start(va,fmt);
    int ret = vprint_log(logs,fmt,va);
    va_end(va);
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

int print_array_write(char_array * array, const char * fmt, ...)
{
    va_list va;
    va_start(va,fmt);
    int ret = vprint_array_write(array,fmt,va);
    va_end(va);
    return ret;
}

int vprint_array_write(char_array * array, const char * fmt, va_list va)
{
    va_list va2;
    va_copy(va2,va);

    size_t new;

    new = vsnprintf(NULL,0,fmt,va);

    array_alloc(array,new + 1);
    
    vsnprintf(array->begin,new + 1,fmt,va2);

    array->end = array->begin + new;
    *array->end = '\0';

    va_end(va);
    va_end(va2);
    
    return new;
}

int print_array_append(char_array * array, const char * fmt, ...)
{
    va_list va;
    va_start(va,fmt);
    int ret = vprint_array_append(array,fmt,va);
    va_end(va);
    return ret;
}

int vprint_array_append(char_array * array, const char * fmt, va_list va)
{
    va_list va2;
    va_copy(va2,va);

    int new = vsnprintf(NULL,0,fmt,va);

    size_t have = count_range(*array);
    size_t len = have + new;

    assert(array->end == NULL || *array->end == '\0');
    assert(array->end == NULL || array->end[0] == '\0');
    assert(array->end == NULL || array->end[-1] != '\0');

    array_alloc(array,len + 1);
    
    vsnprintf(array->end,new + 1,fmt,va2);

    array->end += new;
    *array->end = '\0';
    
    assert(array->end == NULL || array->end[0] == '\0');
    assert(array->end == NULL || array->end[-1] != '\0');

    va_end(va);
    va_end(va2);
    
    return new;
}

int
_log_here(int logs, const char * title, const char * file, const char * func, const char * fmt, ...)
{
    int ret;
    char_array message = {};
    ret = print_array_write(&message,"%s: %s: %s: ",title,file,func);

    va_list va;

    va_start(va,fmt);
    ret += vprint_array_append(&message,fmt,va);
    va_end(va);

//    fputs(message.begin,stderr);
//    fputc('\n',stderr);
    print_log(logs,"%s",message.begin);
    free(message.begin);
    
    return ret;
    
}
