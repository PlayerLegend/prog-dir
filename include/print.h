#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

int
_print_error(const char * file, const char * func, const char * fmt, ...);

#define print_error(fmt,...)				\
    _print_error(__FILE__,__func__,fmt,##__VA_ARGS__)

#define print_error_return(fmt,...) { print_error(fmt,##__VA_ARGS__); return -1; }

int print_buffer_write(char ** string, const char * fmt, ...);
int print_buffer_append(char ** string, const char * fmt, ...);
