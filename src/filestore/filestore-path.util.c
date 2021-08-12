#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../buffer_io/buffer_io.h"
#include "../log/log.h"
#include "../keyargs/keyargs.h"
#include "../vluint/vluint.h"
#include "../metabase/metabase.h"
#include "../metahash/metahash.h"
#include "filestore.h"

bool get_path (const char * arg)
{
    static buffer_unsigned_char arg_decoded = {0};
    buffer_rewrite (arg_decoded);

    range_const_char arg_encoded = { .begin = arg, .end = arg };

    while (*arg_encoded.end && !isspace (*arg_encoded.end))
    {
	arg_encoded.end++;
    }
    
    metabase_decode (&arg_decoded, &arg_encoded);

    static buffer_char result = {0};
    buffer_rewrite (result);

    if (!filestore_path (&result, &arg_decoded.range_cast.const_cast))
    {
	return false;
    }

    printf ("%s\n", result.begin);

    return true;
}

int main(int argc, char *argv[])
{
    if (argc >= 2)
    {
	for (int i = 1; i < argc; i++)
	{
	    if (!get_path (argv[i]))
	    {
		abort();
	    }
	}
    }
    else
    {
	range_char line;
	buffer_char buffer = {0};
	
	while (buffer_getline_fd(.line = &line, .read.buffer = &buffer, .read.fd = STDIN_FILENO))
	{
	    if (!get_path (line.begin))
	    {
		abort();
	    }
	}

	free (buffer.begin);
    }

    return 0;
}
