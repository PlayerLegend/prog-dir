#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "range.h"
#endif

size_t range_strstr (const range_const_char * haystack, const range_const_char * needle);

size_t range_strchr (const range_const_char * haystack, char needle);

bool range_streq (const range_const_char * a, const range_const_char * b);

bool range_streq_string (const range_const_char * a, const char * b);

size_t range_atozd (size_t * restrict output, const range_const_char * restrict input);
