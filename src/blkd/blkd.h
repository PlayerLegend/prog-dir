#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#endif

typedef uint64_t blkd_size;
typedef union blkd_mem_digest blkd_mem_digest;
union blkd_mem_digest {
    uint8_t uint8[32];
    uint32_t uint32[8];
    uint64_t uint64[4];
};

typedef char blkd_group_name[16];

#define BLKD_SECTOR_SIZE 4096
#define BLKD_MAGIC *((blkd_size*)(const char[]) { 'b', 'l', 'k', 'd', 'h', 'e', 'a', 'd' })
#define BLKD_HEADER_VERSION 1

typedef struct blkd_extent blkd_extent;
struct blkd_extent {
    blkd_size begin;
    blkd_size end;
};

typedef struct blkd_node blkd_node;
struct blkd_node {
    uint64_t size;
    blkd_extent alloc;
    unsigned char metahash[];
};

typedef enum blkd_type blkd_type;
enum blkd_type {
    BLKD_TYPE_DATA,
    BLKD_TYPE_FILE,
    BLKD_TYPE_DIR,
    BLKD_TYPE_NODES,
    BLKD_TYPE_PUBKEY_LIST,
};

typedef struct {
    union {
	struct {
	    blkd_size magic;
	    blkd_size version;
	    blkd_size share_groups;
	    blkd_size content_size;
	    union {
		blkd_type type;
		blkd_size _type_reserve;
	    };
	    blkd_size wrote_time;
	};
	char _reserve[1024];
    };
    unsigned char header_metadigest[1024];
    unsigned char data_metadigest[2048];
}
    blkd_header;

typedef union {
    char _reserve[BLKD_SECTOR_SIZE];
    struct {
	blkd_size item_count;
	unsigned char items_start;
	// item format: blkd_size size_of_rest; unsigned char metahash[]; char null_terminated_name[];
    };
}
    blkd_dir_sector;

bool blkd_mkfs (int fd);
