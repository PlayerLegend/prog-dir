#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../json/json.h"
#include "gltf.h"
#include "../log/log.h"

int main (int argc, char * argv[])
{
    if (argc != 2)
    {
	log_error ("usage: %s [glb-file]", argv[0]);
	return 1;
    }

    glb_toc toc;
    FILE * file = fopen (argv[1], "r");
    bool success = glb_toc_load_file (&toc, file);
    fclose (file);
    
    if (!success)
    {
	return 1;
    }

    log_normal ("glb version: %u", toc.header->version);
    log_normal ("file size: %u", toc.header->length);
    log_normal ("json size: %u", toc.json->length);
    log_normal ("bin size: %u", toc.bin->length);

    log_normal ("json data: %.*s", toc.json->length, (const char*)toc.json->data); 

    return 0;
}
