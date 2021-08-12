#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "arch.h"
#endif

typedef struct machine_code machine_code;
struct machine_code
{
    buffer_instruction instructions;
    buffer_char constants;
};

bool write_machine_code (FILE * file, machine_code * code);
bool read_machine_code (machine_code * code, FILE * file);
