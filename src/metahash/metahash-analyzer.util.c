#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../vluint/vluint.h"
#include "metahash.h"
#include "../metabase/metabase.h"
#include "../buffer_io/buffer_io.h"
#include "../log/log.h"

static bool analyze_digest (const range_const_char * digest_armor)
{
    static buffer_unsigned_char decode = {0};
    buffer_rewrite (decode);
    
    static buffer_char encode = {0};
    
    log_normal ("input: %.*s", (int) range_count (*digest_armor), digest_armor->begin);

    if (!metabase_decode (&decode, digest_armor))
    {
	goto fail;
    }

    range_const_unsigned_char rest = decode.range_cast.const_cast;
    range_const_unsigned_char digest;

    metahash_id id;
    vluint_result version;

    //log_debug ("first: %c(%d)", *rest.begin, *rest.begin);

    while (!range_is_empty (rest))
    {
	if (!metahash_unwrap(.input = &rest, .rest = &rest, .id = &id, .version = &version, .digest = &digest))
	{
	    goto fail;
	}
	
	buffer_rewrite (encode);

	metabase_encode (&encode, &digest);

	log_normal ("\tversion %zu, type %s (%x): %.*s", version, metahash_name(id), id, (int)range_count(encode), encode.begin);
    }

    log_normal ("");
    
    return true;

fail:
    return false;
}

static void truncate_spaces (range_const_char * digest_armor)
{
    const char * end = digest_armor->begin;
    while (*end && !isspace(*end))
    {
	end++;
    }
    
    digest_armor->end = end;
}

int main(int argc, char *argv[])
{
    range_const_char digest_armor;
    if (argc >= 2)
    {
	for (int i = 1; i < argc; i++)
	{
	    digest_armor = (range_const_char) { .begin = argv[i],
						.end = digest_armor.begin + strlen (digest_armor.begin) };

	    truncate_spaces (&digest_armor);
	    
	    if (!analyze_digest (&digest_armor))
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
	    digest_armor = line.const_cast;
	    truncate_spaces (&digest_armor);
	    
	    if (!analyze_digest (&digest_armor))
	    {
		abort();
	    }
	}

	free (buffer.begin);
    }

    return 0;
}
