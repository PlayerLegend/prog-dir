#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "arch.h"
#include "machine-code.h"
#endif

typedef union vm_register vm_register;
union vm_register { char _min[4]; char _char; vm_float _float; vm_int _int; vm_uint _uint; };

typedef struct vm_state vm_state;
struct vm_state
{
    struct range(instruction) instructions;
    range_char memory;
    
    vm_address instruction_pointer;
    vm_address frame;
    vm_register result;
};

bool vm_frame (vm_state * state);
bool vm_init (vm_state * state, machine_code * code, size_t memory);
