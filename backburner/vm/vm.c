#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "arch.h"
#include "machine-code.h"
#include "vm.h"
#include "../log/log.h"

#define fatal(...) { log_error (__VA_ARGS__); goto fail; }

bool vm_frame (vm_state * state)
{
    instruction * current_instruction;
    vm_register * arg;

    vm_address memory_size = range_count (state->memory) - sizeof (vm_register);
    vm_address memory_mod;

    vm_address instructions_count = range_count (state->instructions);
    vm_address instruction_address;

    vm_address instruction_arg_address;
    
    while (true)
    {
	instruction_address = state->instruction_pointer++ % instructions_count;
	current_instruction = state->instructions.begin + instruction_address;

	/*if (state->frame >= memory_size) // todo: check this only when frame is changed
	{
	    fatal ("Out of memory");
	    }*/

	memory_mod = memory_size - state->frame;
	instruction_arg_address = ( current_instruction->arg + memory_mod ) % memory_mod;
	arg = (void*)(state->memory.begin + instruction_arg_address);

	log_normal ("instruction pointer: %d, opcode %d", state->instruction_pointer - 1, current_instruction->opcode);

	switch (current_instruction->opcode)
	{
	case OP_EXIT:
	    goto success;

	    // int
	    
	case OP_INT_LOAD:
	    state->result._int = arg->_int;
	    break;

	case OP_INT_STORE:
	    arg->_int = state->result._int;
	    break;

	case OP_INT_ADD:
	    state->result._int += arg->_int;
	    break;

	case OP_INT_SUBTRACT:
	    state->result._int -= arg->_int;
	    break;

	case OP_INT_MULTIPLY:
	    state->result._int *= arg->_int;
	    break;

	case OP_INT_DIVIDE:
	    state->result._int /= arg->_int;
	    break;

	    // float
	    
	case OP_FLOAT_LOAD:
	    state->result._float = arg->_float;
	    break;

	case OP_FLOAT_STORE:
	    arg->_float = state->result._float;
	    break;

	case OP_FLOAT_ADD:
	    state->result._float += arg->_float;
	    break;

	case OP_FLOAT_SUBTRACT:
	    state->result._float -= arg->_float;
	    break;

	case OP_FLOAT_MULTIPLY:
	    state->result._float *= arg->_float;
	    break;

	case OP_FLOAT_DIVIDE:
	    state->result._float /= arg->_float;
	    break;

	    // conversion

	case OP_ITOF:
	    state->result._float = state->result._int;
	    break;

	case OP_FTOI:
	    state->result._int = state->result._float;
	    break;

	case OP_JUMP_IF_NOT:
	    log_debug ("Check: %d", state->result._int);
	    
	    if (state->result._int)
	    {
		log_debug ("Jump cancelled");
		break;
	    }

	    // fall through
	    
	case OP_JUMP:
	    log_debug ("jump %d, %f", current_instruction->arg, state->result._float);
	    state->instruction_pointer = current_instruction->arg;
	    break;
	    
	default:
	    fatal ("Unrecognized opcode %zu at instruction %zu", current_instruction->opcode, state->instruction_pointer - 1);
	}
    }

success:

    return true;
								     
fail:
    return false;
}

bool vm_init (vm_state * state, machine_code * code, size_t memory)
{
    if ((size_t)range_count (code->constants) >= memory)
    {
	fatal ("Not enough memory to allocate constants");
    }
    
    range_calloc (state->memory, memory);

    memcpy (state->memory.begin, code->constants.begin, range_count (code->constants) * sizeof (*code->constants.begin));

    size_t instruction_count = range_count (code->instructions);

    range_calloc (state->instructions, instruction_count);

    memcpy (state->instructions.begin, code->instructions.begin, instruction_count * sizeof (*code->instructions.begin));

    return true;

fail:
    return false;
}
