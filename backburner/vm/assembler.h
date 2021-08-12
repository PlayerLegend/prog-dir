#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../immutable/immutable.h"
#include "../paren-parser/paren-parser.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "arch.h"
#include "vm.h"
#endif

bool assembler (machine_code * result, table_string * table, paren_atom * root);
