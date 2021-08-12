#ifndef FLAT_INCLUDES
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "arch.h"
#include "machine-code.h"
#endif

bool write_machine_code (FILE * file, machine_code * code)
{
    vm_address header[2] = { range_count (code->constants), range_count (code->instructions) };
    size_t count;

    if (2 > fwrite (header, sizeof(*header), 2, file))
    {
	perror ("fwrite");
	return false;
    }

    count = range_count (code->constants);
    
    if (count != fwrite (code->constants.begin, sizeof(*code->constants.begin), count, file))
    {
	perror ("fwrite");
	return false;
    }

    count = range_count (code->instructions);
    
    if (count > fwrite (code->instructions.begin, sizeof(*code->instructions.begin), count, file))
    {
	perror ("fwrite");
	return false;
    }

    return true;
}

bool read_machine_code (machine_code * code, FILE * file)
{
    vm_address header[2];

    size_t count;

    if (2 > fread (header, sizeof(*header), 2, file))
    {
	perror ("fread");
	return false;
    }

    buffer_resize (code->constants, header[0]);
    buffer_resize (code->instructions, header[1]);

    count = range_count (code->constants);
    
    if (count > fread (code->constants.begin, sizeof (*code->constants.begin), count, file))
    {
	perror ("fread");
	return false;
    }

    count = range_count (code->instructions);
    
    if (count > fread (code->instructions.begin, sizeof (*code->instructions.begin), count, file))
    {
	perror ("fread");
	return false;
    }

    return true;
}
