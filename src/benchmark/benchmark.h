#ifndef FLAT_INCLUDES
#include <sys/time.h>
#include <stddef.h>
#define FLAT_INCLUDES
#endif

/** @file benchmark/benchmark.h

    This file provides functions for benchmarking the execution time of a program section.
*/

typedef unsigned long long usec; ///< An integer for storing microseconds

struct timeval start; ///< The start time of the current test

inline static void benchmark_start() /// Begins a test
{
    gettimeofday (&start, NULL); 
}

inline static usec benchmark_delta() /// Finishes a test and returns the microseconds elapsed since its beginning
{
    struct timeval t;
    gettimeofday (&t, NULL);
    return 1e6 * (t.tv_sec - start.tv_sec) + (t.tv_usec - start.tv_usec);
} 
