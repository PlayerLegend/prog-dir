#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "blkd.h"
#endif

bool blkd_direct_read (blkd_block * block, blkd_size offset);
bool blkd_direct_write (const blkd_block * block, blkd_size offset);
