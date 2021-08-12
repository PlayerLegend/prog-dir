#ifndef FLAT_INCLUDES
#include <stdint.h>
#define FLAT_INCLUDES
#endif

#define BLKD_BLOCK_SIZE 4096

typedef uint64_t blkd_size;

typedef struct blkd_extent blkd_extent;
struct blkd_extent {
    blkd_size begin;
    blkd_size size;
};

#define LOG_BLOCK_COUNT 16
#define LOG_EVENT_SIZE_BITS (8 * sizeof(blkd_extent) + 1)
#define LOG_EVENT_COUNT ( (LOG_BLOCK_COUNT * BLKD_BLOCK_SIZE) / LOG_EVENT_SIZE_BITS )

typedef struct blkd_log blkd_log;
struct blkd_log {
    blkd_extent extent[LOG_EVENT_COUNT];
    unsigned char is_alloc[LOG_EVENT_COUNT / 8];
};
