#include <assert.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>
char *realpath(const char *path, char *resolved_path);
#define FLAT_INCLUDES
#include "immutable.h"
#include "../array/range.h"

#define TABLE_STRING

#include "../table2/table.h"
#include "../table2/table-string.h"



struct immutable_namespace
{
    table_string table;
    pthread_mutex_t mutex;
};

static immutable_namespace _default_namespace = { .mutex = PTHREAD_MUTEX_INITIALIZER };

const char * immutable_string (immutable_namespace * namespace, const char * input)
{
    assert (input);

    if (!namespace)
    {
	namespace = &_default_namespace;
    }
    
    const char * ret;
    pthread_mutex_lock (&namespace->mutex);
    ret = table_string_include (namespace->table, input)->query.key;
    pthread_mutex_unlock (&namespace->mutex);
    return ret;
}

const char * immutable_path (immutable_namespace * namespace, const char * path)
{
    assert (path);
    
    if (!namespace)
    {
	namespace = &_default_namespace;
    }
    
    const char * ret;
    static char resolved[PATH_MAX];
    assert (path);
    pthread_mutex_lock (&namespace->mutex);
    
    if (realpath (path, resolved))
    {
	ret = table_string_include (namespace->table, resolved)->query.key;
    }
    else
    {
	ret = table_string_include (namespace->table, "")->query.key;
	perror (path);
    }
    pthread_mutex_unlock (&namespace->mutex);
    return ret;
}
