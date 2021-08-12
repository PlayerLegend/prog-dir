#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../immutable/immutable.h"
#include "arch.h"
#endif

typedef struct keywords keywords;
struct keywords
{
    table_string * table;
    
    const char * _float;
    const char * _int;
    
    const char * name;
    
    const char * exit;

    const char * ifelse;
    
    const char * load_int;
    const char * store_int;

    const char * load_float;
    const char * store_float;

    const char * load_char;
    const char * store_char;

    const char * scope_local;
    const char * scope_global;
    
    const char * add_int;
    const char * add_float;

    const char * subtract_int;
    const char * subtract_float;

    const char * multiply_int;
    const char * multiply_float;

    const char * divide_int;
    const char * divide_float;

    const char * address;

    const char * sum;
};

bool is_vm_address (const char * text);
bool read_vm_address (vm_address * result, const char * text);

bool is_vm_float (const char * text);
bool read_vm_float (vm_float * result, const char * text);

bool is_vm_int (const char * text);
bool read_vm_int (vm_int * result, const char * text);

void init_keywords (keywords * result, table_string * table);

bool parse_written_constant (buffer_char * result, const char ** type, keywords * keywords, const char * text);
