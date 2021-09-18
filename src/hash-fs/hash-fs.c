#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "hash-fs.h"
#include "../keyargs/keyargs.h"
#include "../vluint/vluint.h"
#include "../metahash/metahash.h"
#include "internal.h"
#include "../log/log.h"

inline static hash_fs_size mod_add (hash_fs_size a, hash_fs_size b, hash_fs_size mod)
{
    assert (a == a % mod);
    assert (b == b % mod);

    hash_fs_size negative_a = mod - a;

    if (b >= negative_a)
    {
	return b - negative_a; // this is the amount that non-modular addition would exceed mod by
    }
    else // otherwise, non-modular addition and modular addition yield the same value
    {
	return a + b;
    }
}

inline static hash_fs_size mod_reference (const hash_fs_digest * digest, hash_fs_size offset, hash_fs_size mod)
{
    hash_fs_size point = offset % mod;

    for (unsigned char i = 0; i < sizeof(digest->ints) / sizeof(digest->ints[0]); i++)
    {
	point = mod_add (point, digest->ints[i] % mod, mod);
    }

    return point;
}

static bool lseek_fs (hash_fs * fs, const hash_fs_digest * digest, hash_fs_size offset)
{
    return -1 == lseek (fs->io.fd,
			HASH_FS_USER_BLOCK_START * HASH_FS_SECTOR_SIZE + mod_reference (digest, offset, hash_fs_user_block_count (fs)),
			SEEK_SET);
}

static bool hash_fs_cache_load (hash_fs * fs, hash_fs_loaded_sector * target, const hash_fs_digest * digest, hash_fs_size offset)
{
    ssize_t size = -1;
    bool seek_success = false;
    
    pthread_mutex_lock (&fs->io.mutex);

    seek_success = lseek_fs (fs, digest, offset);

    if (!seek_success)
    {
	perror ("hash_fs_cache_load");
    }
    else
    {
	size = read (fs->io.fd, target->contents.input, sizeof (target->contents.input));
    }
	
    pthread_mutex_unlock (&fs->io.mutex);

    if (!seek_success)
    {
	log_fatal ("Could not seek to requested sector");
    }
    
    if (size < 0)
    {
	perror ("read");
	log_fatal ("Reading the requested sector resulted in an error");
    }

    if (size != sizeof (target->contents.input))
    {
	log_fatal ("Reading the requested sector resulted in a truncated read");
    }
    
    target->contents.header.digest = *digest;

    target->is_loaded = true;
    
    return true;
    
fail:
    target->is_loaded = false;
    return false;
}

static bool hash_fs_digest_equals (const hash_fs_digest * a, const hash_fs_digest * b)
{
    return 0 == memcmp (a->bytes, b->bytes, sizeof(a->bytes)) && a->type_index == b->type_index;
}

static bool hash_fs_cache_access (hash_fs_loaded_sector ** return_target, hash_fs * fs, const hash_fs_digest * digest, hash_fs_size offset)
{
    hash_fs_loaded_sector * target = fs->sector_table.begin + mod_reference (digest, offset, range_count (fs->sector_table));

    pthread_mutex_lock (&target->mutex);

    if (target->is_loaded)
    {
	if (hash_fs_digest_equals (&target->contents.header.digest, digest))
	{
	    *return_target = target;
	    return true;
	}
	else
	{
	    pthread_mutex_unlock (&target->mutex);
	    *return_target = NULL;
	    return true;
	}
    }
    else
    {
	if (!hash_fs_cache_load (fs, target, digest, offset))
	{
	    log_fatal ("Could not load the requested sector");
	}

	*return_target = target;
	
	return true;
    }
    
fail:
    pthread_mutex_unlock (&target->mutex);
    return false;
}
