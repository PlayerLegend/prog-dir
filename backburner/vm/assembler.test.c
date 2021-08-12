#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../immutable/immutable.h"
#include "../paren-parser/paren-parser.h"
#include "../paren-parser/paren-preprocessor.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../log/log.h"
#include "arch.h"
#include "machine-code.h"
#include "assembler.h"

const char * name_instruction (instruction * instruction)
{
    switch (instruction->opcode)
    {
    case OP_EXIT:
	return "exit";

	
    case OP_FLOAT_ADD:
	return "+float";

    case OP_FLOAT_SUBTRACT:
	return "-float";
	
    case OP_FLOAT_DIVIDE:
	return "/float";


    case OP_FLOAT_MULTIPLY:
	return "*float";

    case OP_FLOAT_STORE:
	return "store-float";

    case OP_FLOAT_LOAD:
	return "load-float";

	
    case OP_INT_ADD:
	return "+int";

    case OP_INT_SUBTRACT:
	return "-int";
	
    case OP_INT_DIVIDE:
	return "/int";


    case OP_INT_MULTIPLY:
	return "*int";

    case OP_INT_STORE:
	return "store-int";

    case OP_INT_LOAD:
	return "load-int";

	
    case OP_JUMP:
	return "jump";

    case OP_JUMP_IF_NOT:
	return "jump-if-not";

    default:
	return "(unknown)";
    }
}

int main (int argc, char * argv[])
{
    assert (argc == 2);

    paren_atom * atom = paren_preprocessor (.filename = argv[1]);

    machine_code mc = {0};

    if (!assembler (&mc, NULL, atom))
    {
	log_error ("Could not assemble output");
	return 1;
    }

    for (int i = 0; i < range_count (mc.instructions); i++)
    {
	fprintf (stderr, "(%d)\t%10s\t%d\n", i, name_instruction(mc.instructions.begin + i), mc.instructions.begin[i].arg);
    }
    
    if (!write_machine_code(stdout, &mc))
    {
	log_error ("Could not write output");
	return 1;
    }

    paren_atom_free (atom);

    return 0;
}
