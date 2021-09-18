#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#define FLAT_INCLUDES
#endif

typedef struct hash_fs hash_fs;
typedef uint64_t hash_fs_size;

hash_fs * hash_fs_new();
bool hash_fs_load (hash_fs * fs, const char * path);
