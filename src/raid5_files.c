#include <stdint.h>

typedef uint8_t fs_drive_t;
typedef uint64_t fs_size_t;
typedef uint32_t block_sum_t;

typedef struct
{
    fs_size_t drive;
    fs_size_t block;
}
    block_id;

typedef struct {
    fs_drive_t drive[3];
    fs_size_t block;
}

typedef block_id block_id_trinity[3];
typedef block_sum_t block_sum_trinity[3];

