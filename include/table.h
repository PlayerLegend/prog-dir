#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "stack.h"
#include "range.h"
#include "array.h"
#include "bucket.h"
#endif

typedef struct {
    bucket_config config;
    array(size_t) deleted_id;
    range(bucket);
}
    open_table;

typedef struct {
    bucket_config config;
    array(size_t) deleted_id;
    range(void*);
}
    closed_table;

