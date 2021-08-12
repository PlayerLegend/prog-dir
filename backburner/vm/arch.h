#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#endif

typedef uint32_t vm_address;
typedef int32_t vm_int;
typedef uint32_t vm_uint;
typedef float vm_float;
//typedef struct location location;
/*struct location {
    union { address address; int32_t intval; uint32_t uintval; float floatval; };
    };*/

typedef enum opcode opcode;
enum opcode {
    OP_EXIT,

    OP_INT_LOAD,
    OP_INT_STORE,
    OP_INT_ADD,
    OP_INT_SUBTRACT,
    OP_INT_MULTIPLY,
    OP_INT_DIVIDE,

    OP_FLOAT_LOAD,
    OP_FLOAT_STORE,
    OP_FLOAT_ADD,
    OP_FLOAT_SUBTRACT,
    OP_FLOAT_MULTIPLY,
    OP_FLOAT_DIVIDE,
    
    OP_ITOF,
    OP_FTOI,

    OP_JUMP,
    OP_JUMP_IF_NOT,
};

typedef struct instruction instruction;
struct instruction {
    vm_address opcode, arg;
};

typedef struct buffer(instruction) buffer_instruction;
//typedef struct buffer(location) buffer_location;

