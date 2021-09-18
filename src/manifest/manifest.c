#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "manifest.h"

bool manifest_read (manifest_item * items, FILE * file)
{
    manifest_item * end = items;

    while (end->key)
    {
	end++;
    }

    size_t count = 0;

    bool first_char = false;

    
}
void manifest_clear (manifest_item * items);
