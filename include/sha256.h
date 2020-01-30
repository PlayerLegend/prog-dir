#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#include <stdbool.h>
#endif

typedef char sha256_armor[64 + 1];
typedef unsigned char sha256[32];

typedef struct {
    FILE * file;
    char * buffer;
    ssize_t buffer_size;
    sha256 sum;
    void * ctx;
    bool success;
}
    sha256_job;

void sha256_makearmor(sha256_armor armor, const sha256 orig);
int sha256_stream(sha256 sum, FILE * file, size_t buffer_size);
int sha256_path(sha256 sum, const char * path, size_t buffer_size);
int sha256_buffer(sha256 sum, const void * buffer, size_t size);
int sha256_partial(sha256_job * job);
void sha256_halt_partial(sha256_job * job);
