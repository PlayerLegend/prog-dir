#ifndef FLAT_INCLUDES
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "arch.h"
#endif

vm_address record_constant (buffer_char * constants, void * constant_value, size_t constant_size);
