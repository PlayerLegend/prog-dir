#ifndef FLAT_INCLUDES

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define FLAT_INCLUDES

#include "range.h"

#endif

enum { LOG_NORMAL = 1 << 0,
       LOG_ERROR = 1 << 1,
       LOG_DEBUG = 1 << 2 };

int
_log_here(int logs, const char * title, const char * file, const char * func, const char * fmt, ...);

int print_buffer_write(char ** string, const char * fmt, ...);
int print_buffer_append(char ** string, const char * fmt, ...);

int vprint_array_append(char_array * array, const char * fmt, va_list va);
int print_array_write(char_array * array, const char * fmt, ...);

int vprint_array_write(char_array * array, const char * fmt, va_list va);
int print_array_append(char_array * array, const char * fmt, ...);

int vprint_log(int logs, const char * fmt, va_list va);
int print_log(int logs, const char * fmt, ...);

int register_log_path(int logs, const char * path);

#define log_normal(fmt,...)				\
    print_log(LOG_NORMAL,fmt "\n",##__VA_ARGS__)

#define log_error(fmt,...)			\
    print_log(LOG_ERROR,fmt "\n",##__VA_ARGS__)

#define log_debug(fmt,...)			\
    _log_here(LOG_DEBUG,"debug",__FILE__,__func__,fmt,##__VA_ARGS__)

#define debug_break(fmt,...) { log_debug(fmt,##__VA_ARGS__); goto ERROR; }
#define error_break(fmt,...) { log_error(fmt,##__VA_ARGS__); goto ERROR; }
