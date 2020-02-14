#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES

#include <stdio.h>
#include <string.h>

#include "stack.h"
#include "array.h"
#include "hash_table.h"
#endif

size_t table_digest_string(const void * key);
int table_equals_string(const void * a, const void * b);
int table_copy_string(void ** dst, const void * src);
void table_free_string(void * key);

#define TABLE_CONFIG_STRING			\
    (table_config){				\
	.gen_digest = table_digest_string,	\
	    .copy = table_copy_string,		\
	    .equals = table_equals_string,	\
	    .free = table_free_string,		\
	    }
