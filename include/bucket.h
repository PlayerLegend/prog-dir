#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stddef.h>
#include <stdbool.h>
#endif

typedef struct {
    void * content;
    size_t digest;
    size_t id;
}
    bucket;

typedef size_t (*bucket_digest)(const void * content);
typedef bool (*bucket_equals)(const void * a, const void * b);
typedef void (*bucket_free)(void * content);
typedef bool (*bucket_copy)(void ** dst, const void * src);

typedef struct {
    bucket_digest digest;
    bucket_equals equals;
    bucket_free free;
    bucket_copy copy;
}
    bucket_config;
