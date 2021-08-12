#ifndef FLAT_INCLUDES
#include <stdint.h>
#define FLAT_INCLUDES
#endif

typedef uint64_t blkd_size;

#define BLKD_SECTOR_SIZE 4096
#define BLKD_FREEMAP_SIZE (BLKD_SECTOR_SIZE * (BLKD_SECTOR_SIZE * 8))
#define BLKD_PBLOCK_COUNT (BLKD_SECTOR_SIZE / sizeof(blkd_size))

typedef unsigned char sha256sum_digest[256 / 8];

typedef struct blkd_map blkd_map;
struct blkd_map
{
    sha256sum_digest checksum;
    unsigned char bits[BLKD_SECTOR_SIZE - sizeof(sha256sum_digest)];
};

typedef struct blkd_tree_branch blkd_tree_branch;
struct blkd_tree_branch
{
    sha256sum_digest checksum;
    blkd_size pointer[(BLKD_SECTOR_SIZE - sizeof(sha256sum_digest)) / sizeof(blkd_size)];
};

typedef struct blkd_tree blkd_tree;
struct blkd_tree
{
    blkd_size indirection_count; // 0: these are leaves, 1: these point to leaves, ...
    blkd_size pointer[4];
};

typedef struct blkd_inode_directory_pair blkd_inode_directory_pair;
struct blkd_inode_directory_pair
{
    blkd_size digest2;
    blkd_size pointer;
};

typedef struct blkd_inode_directory blkd_inode_directory;
struct blkd_inode_directory
{
    sha256sum_digest digest;
    blkd_inode_directory_pair pairs[(BLKD_SECTOR_SIZE - sizeof(sha256sum_digest)) / (2 * 8)];
};

typedef struct blkd_inode blkd_inode;
struct blkd_inode
{
    union {
	struct {
	    blkd_tree owner_ids;
	    blkd_tree data_blocks;
	};

	unsigned char reserved[BLKD_SECTOR_SIZE / 2];
    };

    unsigned char digest[BLKD_SECTOR_SIZE / 2];
};

struct blkd_data_region_header
{
    sha256sum_digest checksum;
    blkd_size size;
};

struct blkd_superblock
{
    sha256sum_digest checksum;
    blkd_size magic_number;
    blkd_tree inode_table; // refers to blkd_inode_directories
    blkd_tree data_region;
};

typedef struct blkd_generic_block blkd_generic_block;
struct blkd_generic_block
{
    sha256sum_digest checksum;
    unsigned char payload[BLKD_SECTOR_SIZE - sizeof(sha256sum_digest)];
};

typedef union blkd_block blkd_block;
union blkd_block
{
    blkd_generic_block generic;
    blkd_map map;
    blkd_tree tree;
    blkd_tree_branch tree_branch;
    blkd_inode_directory directory;
};

typedef struct blkd_system blkd_system;
