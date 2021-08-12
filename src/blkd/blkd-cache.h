#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#define FLAT_INCLUDES
#include "blkd.h"
#endif

bool blkd_cache_resize(blkd_size size);
blkd_block * blkd_cache_open(blkd_system * system, blkd_size offset);
