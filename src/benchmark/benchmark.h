#ifndef FLAT_INCLUDES
#include <sys/time.h>
#include <stddef.h>
#define FLAT_INCLUDES
#endif

typedef unsigned long long usec;

struct timeval start;

static void benchmark_start()
{
    gettimeofday (&start, NULL); 
}

static usec benchmark_delta()
{
    struct timeval t;
    gettimeofday (&t, NULL);
    return 1e6 * (t.tv_sec - start.tv_sec) + (t.tv_usec - start.tv_usec);
}
