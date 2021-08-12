#define FLAT_INCLUDES

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include "../array/range.h"
#include "../list/list.h"

#include "blkd.h"
#include "blkd-cache-io.h"

typedef struct block_id block_id;
struct block_id
{
    size_t system_id, index;
};

#define table_key block_id
#define table_key_copy(dst,src) (dst) = (src)
#define table_key_hash(digest,src) (digest) = ((src).index + ((src).system_id << (sizeof(digest) / 2)))
#define table_key_equals(a,b) ((a).index == (b).index && (a).system_id == (b).system_id)
#define table_key_free(key)
#define TABLE_VALUE struct { bool modified; blkd_block block; }
#include "../table/table.h"

static struct {
    pthread_mutex_t mutex;
    size_t loaded_block_count;
    table block_table;
}
    loadblock_state = { .mutex = PTHREAD_MUTEX_INITIALIZER };

const blkd_block * blkd_load_containing_block_ro (blkd_system * system, blkd_size offset)
{
    return NULL;
}

blkd_block * blkd_load_containing_block_rw (blkd_system * system, blkd_size offset)
{
    return NULL;
}
