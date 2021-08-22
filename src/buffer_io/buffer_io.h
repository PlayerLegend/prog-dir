#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#endif

#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

#define BUFFER_IO_SMALL (1 << 20)

long int buffer_printf(buffer_char * buffer, const char * str, ...);
long int buffer_printf_append(buffer_char * buffer, const char * str, ...);

keyargs_declare(long int, buffer_read, buffer_char * buffer; size_t max_buffer_size; size_t initial_alloc_size; int fd;);
#define buffer_read(...) keyargs_call(buffer_read, __VA_ARGS__)

keyargs_declare(long int, buffer_write, range_const_char * buffer; size_t * wrote_size; int fd;);
#define buffer_write(...) keyargs_call(buffer_write, __VA_ARGS__)

keyargs_declare(bool, buffer_getline_fd,
		range_char * line;
		const char * sep;
		size_t protect_size;
		bool init;
		keyargs_struct_name(buffer_read) read;);

#define buffer_getline_fd(...) keyargs_call(buffer_getline_fd, __VA_ARGS__)

void buffer_getline_end (size_t protect_size, const char * sep, range_char * line, buffer_char * buffer);

#define buffer_getline_init(buffer_p) { (buffer_p)->end = (buffer_p)->begin; }

void buffer_strcpy (buffer_char * to, const char * input);
