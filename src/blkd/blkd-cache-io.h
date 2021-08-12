#ifndef FLAT_INCLUDES
#include <stdint.h>
#define FLAT_INCLUDES
#include "blkd.h"
#endif

const blkd_block * blkd_load_containing_block_ro (blkd_system * system, blkd_size offset);
blkd_block * blkd_load_containing_block_rw (blkd_system * system, blkd_size offset);
