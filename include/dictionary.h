#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#include <stdio.h>
#include "stack.h"
#include "array.h"
#include "index_map.h"
#include "hash_table.h"
#endif

#define dictionary(type) \
    struct { table keys; index_map(type) values; }

#define dictionary_table(dictp)			\
    ( (dictp)->keys )

#define dictionary_include(dictp,key)		\
    ( table_include(&(dictp)->keys,key) )

#define dictionary_access_index(dictp,index)		\
    ( index_map_access(&(dictp)->values,index) )

#define dictionary_access_key(dictp,key)		\
    dictionary_access_index(dictp,table_include(&(dictp)->keys,key))

#define dictionary_indexof_key(dictp,key)	\
    (table_include(&(dictp)->keys,key))

#define dictionary_indexof_value(dictp,valuep)	\
    (valuep - (dictp)->values.begin)

#define dictionary_keyof_index(dictp,index)	\
    table_keyof_index(&(dictp)->keys,index)

#define dictionary_keyof_value(dictp,valuep)	\
    dictionary_keyof_index(dictp,dictionary_indexof_value(dictp,valuep))

#define dictionary_keycount(dictp)				\
    ((dictp)->keys.key.end - (dictp)->keys.key.end)

#define dictionary_config(dictp)		\
    ((dictp)->keys.config)
