#ifndef FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef struct {
    const char * key;
    buffer_char value;
}
    manifest_item;

bool manifest_read (manifest_item * items, FILE * file);
void manifest_clear (manifest_item * items);
