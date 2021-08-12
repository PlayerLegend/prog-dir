#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define FLAT_INCLUDES

#include "../array/range.h"
#include "../array/buffer.h"

#include "metabase.h"

#include "../log/log.h"
#include "../buffer_io/buffer_io.h"

static void print_mbti_personality (buffer_char * output, unsigned char input)
{
    assert (input < 16);

    *buffer_push(*output) = input & 1 ? 'E' : 'I';
    *buffer_push(*output) = input & 2 ? 'S' : 'N';
    *buffer_push(*output) = input & 4 ? 'F' : 'T';
    *buffer_push(*output) = input & 8 ? 'J' : 'P';
}

void metabase_encode_mbti(buffer_char * output, range_const_unsigned_char * input)
{
    buffer_rewrite(*output);
    *buffer_push(*output) = METABASE_MBTI;

    const unsigned char * c;

    for_range (c, *input)
    {
	print_mbti_personality(output, *c % 16);
	print_mbti_personality(output, *c / 16);
    }
}
