#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "hash-fs.h"
#include "../keyargs/keyargs.h"
#include "../vluint/vluint.h"
#include "../metahash/metahash.h"
#endif

#define HASH_FS_SECTOR_SIZE (2 << 14)
#define HASH_FS_SECTOR_CHECKSUM_SIZE 64

typedef struct {
    union {
	uint8_t bytes[64];
	uint64_t ints[64 / sizeof(uint64_t)];
    };

    uint8_t type_index; ///< indexes the checksum_type_array located in the filesystem superblock
}
    hash_fs_digest;

typedef struct {
    uint16_t input_length; ///< The length of the input for this sector
    hash_fs_size miss_count; ///< The maximum number of misses to try when looking from this sector's index. This should be grown or shrunk accordingly when sectors are added and deleted.
    hash_fs_digest digest; ///< checksum and content identifier for the sector's input
}
    hash_fs_sector_header;

typedef struct {
    hash_fs_sector_header header;
    uint8_t input[HASH_FS_SECTOR_SIZE - sizeof(hash_fs_sector_header)];
}
    hash_fs_sector;

typedef struct {
    uint8_t checksum[HASH_FS_SECTOR_CHECKSUM_SIZE];
    uint8_t checksum_type_index;
    hash_fs_size checksum_type_array[256];
}
    hash_fs_superblock; ///< filesystem metadata is stored here

typedef struct {
    hash_fs_size index;
    bool is_loaded;
    pthread_mutex_t mutex;
    hash_fs_sector contents;
}
    hash_fs_loaded_sector;

range_typedef(hash_fs_loaded_sector, hash_fs_loaded_sector);

struct hash_fs {
    struct {
	int fd;
	pthread_mutex_t mutex;
    }
	io;
    
    hash_fs_size block_count;
    range_hash_fs_loaded_sector sector_table;
};

inline static hash_fs_size hash_fs_user_block_count (hash_fs * fs)
{
    return fs->block_count - 2;
}

#define HASH_FS_USER_BLOCK_START 1
