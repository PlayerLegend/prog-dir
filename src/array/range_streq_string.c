#include <stddef.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "range.h"
#include "string.h"
#include "../libc/string.h"

bool range_streq_string (const range_const_char * a, const char * b)
{
    range_const_char b_range = { .begin = b, .end = b + strlen (b) };

    return range_streq(a, &b_range);
}
