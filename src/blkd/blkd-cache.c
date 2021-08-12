#define FLAT_INCLUDES

#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>

#include "../array/range.h"
#include "../array/buffer.h"
#include "../list/list.h"

#include "blkd.h"
#include "blkd-cache-io.h"

typedef struct block_id block_id;
struct block_id
{
    blkd_system * system;
    size_t index;
};

#define table_key block_id
#define table_key_copy(dst,src) (dst) = (src)
#define table_key_hash(digest,src) (digest) = ((src).index + ((size_t)(src).system << (sizeof(digest) / 2)))
#define table_key_equals(a,b) ((a).index == (b).index && (a).system == (b).system)
#define table_key_free(key)
#define TABLE_VALUE struct { bool modified; blkd_block block; }
#include "../table/table.h"

static struct {
    pthread_mutex_t mutex;

    struct buffer(unsigned char) use_weights;
    struct buffer(blkd_block) blocks;
    
    table block_table;
}
    cache_state = { .mutex = PTHREAD_MUTEX_INITIALIZER };

static blkd_size choose_block_to_free()
{
    srand(time(NULL));
    blkd_size test_index;
    unsigned char test_weight;

    blkd_size use_index = rand() % range_count(cache_state.blocks);
    unsigned char use_weight = cache_state.use_weights.begin[use_index];

    for (blkd_size i = 0; i < 10; i++)
    {
	test_index = rand() % range_count(cache_state.blocks);
	test_weight = cache_state.use_weights.begin[test_index];

	if (test_weight < use_weight)
	{
	    use_index = test_index;
	    use_weight = test_weight;
	}
    }

    return use_index;
}
bool blkd_cache
