#include <stdarg.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "log.h"

void _log (log_level level, const char * fmt, ...)
{
    FILE * file = level == LOG_NORMAL ? stdout : stderr;

    va_list va;
    va_start (va, fmt);
    vfprintf (file, fmt, va);
    va_end (va);
}
