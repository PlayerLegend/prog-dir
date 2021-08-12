#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "arch.h"
#include "machine-code.h"
#include "vm.h"
#include "../log/log.h"

int main (int argc, char * argv[])
{
    assert (argc == 2);

    FILE * file = fopen (argv[1], "r");

    if (!file)
    {
	perror (argv[1]);
	return 1;
    }

    machine_code mc = {0};
    
    if (!read_machine_code(&mc, file))
    {
	fclose (file);
	log_error ("Failed to read machine code");
	return 1;
    }
    
    /*for (int i = 0; i < range_count (mc.instructions); i++)
    {
	fprintf (stderr, "%d\t%d\n", mc.instructions.begin[i].opcode, mc.instructions.begin[i].arg);
	}*/

    vm_state state = {0};
    if (!vm_init (&state, &mc, 4e6))
    {
	return 1;
    }

    if (!vm_frame (&state))
    {
	return 1;
    }

    printf ("result: %d(int) %f(float)\n", state.result._int, state.result._float);

    return 0;
}
