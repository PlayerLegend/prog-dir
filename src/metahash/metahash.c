#include <assert.h>
#include <gcrypt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>

#define FLAT_INCLUDES

#include "../keyargs/keyargs.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../buffer_io/buffer_io.h"
#include "../vluint/vluint.h"
#include "metahash.h"
#include "../log/log.h"

// header

#define METAHASH_VERSION 0

#ifndef NDEBUG
static bool verify_vluint(const range_const_unsigned_char * vluint, vluint_result verify)
{
    vluint_result have;
    range_const_unsigned_char have_vluint;
    if (!vluint_read (.result = &have, .input = vluint, .vluint = &have_vluint))
    {
	return false;
    }

    if (have != verify)
    {
	log_error ("vluint failed to verify: %zu != %zu, [length: %zu]", have, verify, range_count (have_vluint));
	return false;
    }

    return true;
}
#endif

keyargs_define(metahash_header_write)
{
    vluint_write (args.output, METAHASH_VERSION);
    vluint_write (args.output, args.id);

    assert (verify_vluint (&args.output->range_cast.const_cast, METAHASH_VERSION));
}

keyargs_define(metahash_header_read)
{
    vluint_result id_result;
    
    if (!vluint_read (.result = args.version, .rest = args.rest, .input = args.input))
    {
	return false;
    }

    if (!vluint_read (.result = &id_result, .rest = args.rest, .input = args.rest))
    {
	return false;
    }

    *args.id = is_metahash_id (id_result);

    if (!*args.id)
    {
	log_error ("Invalid metahash id %zu (%x), got version %zu", id_result, id_result, *args.version);
	return false;
    }
    
    return true;
}

// gcrypt hash wrappers

int convert_metahash_id_to_gcry_id (metahash_id id)
{
    switch (id)
    {
    case METAHASH_SHA2_256:
	return GCRY_MD_SHA256;
	    
    case METAHASH_SHA2_512:
	return GCRY_MD_SHA512;

    case METAHASH_MD5_128:
	return GCRY_MD_MD5;
        
    case METAHASH_SHA1:
	return GCRY_MD_SHA1;
	
    case METAHASH_SHA3_512:
	return GCRY_MD_SHA3_512;
	
    case METAHASH_SHA3_384:
	return GCRY_MD_SHA3_384;
	
    case METAHASH_SHA3_256:
	return GCRY_MD_SHA3_256;
	
    case METAHASH_SHA3_224:
	return GCRY_MD_SHA3_224;
	
    case METAHASH_SHAKE_128:
    case METAHASH_SHAKE_256:
    case METAHASH_KECCAK_224:
    case METAHASH_KECCAK_256:
    case METAHASH_KECCAK_384:
    case METAHASH_MURMUR3_128:
    case METAHASH_MURMUR3_32:
	log_fatal ("Hash unimplemented (%d)", id);
	
    case METAHASH_IDENTITY:
	log_fatal ("Input to gcry id converter is metahash identity");
    }

    log_fatal ("Invalid id (%d)", id);
fail:
    return 0;
}

bool init_context (gcry_md_hd_t * hd, const range_const_metahash_id * use_hashes)
{
    gcry_error_t err;

    if ( (err = gcry_md_open (hd, 0, 0)) )
    {
	log_error ("gcry_md_open: %s", gcry_strerror (err));
	log_fatal ("Could not create context");
    }

    const metahash_id * i;
    int gcry_id;

    for_range (i, *use_hashes)
    {
	gcry_id = convert_metahash_id_to_gcry_id(*i);

	if (!gcry_id)
	{
	    log_fatal ("Could not convert metahash id for gcrypt");
	}

	if ( (err = gcry_md_enable (*hd, gcry_id)) )
	{
	    log_fatal ("Could not enable a hash (gcrypt id: %d) (metahash id: %d)", gcry_id, *i);
	}
    }

    return true;

fail:
    gcry_md_close (*hd);
    return false;
}

bool final_context (buffer_unsigned_char * output, gcry_md_hd_t hd, const range_const_metahash_id * use_hashes)
{
    const metahash_id * i;
    int gcry_id;

    size_t old_size;
    size_t new_size;

    for_range (i, *use_hashes)
    {
	gcry_id = convert_metahash_id_to_gcry_id(*i);

	if (!gcry_id)
	{
	    log_fatal ("Could not convert metahash id for gcrypt");
	}

	old_size = range_count (*output);
	new_size = old_size + gcry_md_get_algo_dlen(gcry_id);

	buffer_resize (*output, new_size);

	memcpy (output->begin + old_size, gcry_md_read (hd, gcry_id), new_size - old_size);
    }

    return true;

fail:
    return false;
}

// is_id
metahash_id is_metahash_id (metahash_id input)
{
    switch (input)
    {
    default:
	return METAHASH_IDENTITY;

    case METAHASH_IDENTITY:
    case METAHASH_SHA1:
    case METAHASH_SHA2_256:
    case METAHASH_SHA2_512:
    case METAHASH_SHA3_512:
    case METAHASH_SHA3_384:
    case METAHASH_SHA3_256:
    case METAHASH_SHA3_224:
    case METAHASH_SHAKE_128:
    case METAHASH_SHAKE_256:
    case METAHASH_KECCAK_224:
    case METAHASH_KECCAK_256:
    case METAHASH_KECCAK_384:
    case METAHASH_MURMUR3_128:
    case METAHASH_MURMUR3_32:
    case METAHASH_MD5_128:
	return input;
    }

    return false;
}

// metahash size
size_t metahash_size (metahash_id input)
{
    switch (input)
    {
    default:
	return 0;
	
    case METAHASH_SHA3_512:
    case METAHASH_SHA2_512:
	return 512 / 8;
		
    case METAHASH_KECCAK_256:
    case METAHASH_SHAKE_256:
    case METAHASH_SHA3_256:
    case METAHASH_SHA2_256:
	return 256 / 8;
        
    case METAHASH_KECCAK_384:
    case METAHASH_SHA3_384:
	return 384 / 8;
        
    case METAHASH_KECCAK_224:
    case METAHASH_SHA3_224:
	return 224 / 8;
	
    case METAHASH_SHA1:
	return 160 / 8;
		
    case METAHASH_MD5_128:
    case METAHASH_MURMUR3_128:
    case METAHASH_SHAKE_128:
	return 128 / 8;
        
    case METAHASH_MURMUR3_32:
	return 32 / 8;

    }

    return 0;
}

// metahash name
const char * metahash_name (metahash_id input)
{
    switch (input)
    {
    default:
    case METAHASH_IDENTITY:
	return "invalid";

    case METAHASH_SHA1:
	return "sha1";
	
    case METAHASH_SHA2_256:
	return "sha2-256";
	
    case METAHASH_SHA2_512:
	return "sha2-512";
	
    case METAHASH_SHA3_512:
	return "sha3-512";
	
    case METAHASH_SHA3_384:
	return "sha3-384";
	
    case METAHASH_SHA3_256:
	return "sha3-256";
	
    case METAHASH_SHA3_224:
	return "sha3-224";
	
    case METAHASH_SHAKE_128:
	return "shake-128";
	
    case METAHASH_SHAKE_256:
	return "shake-256";
	
    case METAHASH_KECCAK_224:
	return "keccak-224";
	
    case METAHASH_KECCAK_256:
	return "keccak-256";
	
    case METAHASH_KECCAK_384:
	return "keccak-384";
	
    case METAHASH_MURMUR3_128:
	return "murmur3-128";
	
    case METAHASH_MURMUR3_32:
	return "murmur3-32";

    case METAHASH_MD5_128:
	return "md5-128";
    }

    return 0;
}

// unwrap

keyargs_define (metahash_unwrap)
{
    metahash_id id;
    buffer_unsigned_char header_rest;
    vluint_result version;
    range_const_unsigned_char input = *args.input;

    if (!metahash_header_read(.id = &id, .version = &version, .rest = &header_rest.range_cast.const_cast, .input = &input))
    {
	log_fatal ("Could not read metahash header");
    }
    
    if (args.id)
    {
	*args.id = id;
    }

    size_t digest_size;
    
    if (args.digest || args.rest)
    {
	digest_size = metahash_size (id);
    }
	
    if (args.digest)
    {
	*args.digest = (range_const_unsigned_char){ .begin = header_rest.begin, .end = header_rest.begin + digest_size };

	if (args.digest->end > input.end)
	{
	    goto fail_too_short;
	}
    }

    if (args.rest)
    {
	*args.rest = (range_const_unsigned_char){ .begin = header_rest.begin + digest_size, .end = input.end };

	if (args.rest->begin > args.rest->end)
	{
	    goto fail_too_short;
	}
    }

    if (args.version)
    {
	*args.version = version;
    }
    
    return true;

fail_too_short:
    log_fatal ("Digest for hash '%s' is too short", metahash_name(id));
    
fail:
    return false;
}

// metahash fd
keyargs_define(metahash_fd)
{
    gcry_md_hd_t context;
    buffer_unsigned_char tmp_buffer = {0};
    int size;
    size_t wrote_size;

    bool context_initialized = false;

    if (!init_context (&context, args.use_hashes))
    {
	log_fatal ("Could not init context");
    }

    context_initialized = true;
    
    while (0 < (size = buffer_read (.fd = args.in_fd, .buffer = (buffer_char*) &tmp_buffer)))
    {
	gcry_md_write (context, tmp_buffer.begin, range_count (tmp_buffer));

	if (args.out_fd >= 0)
	{
	    wrote_size = 0;
	    while (0 < (size = buffer_write (.fd = args.out_fd, .buffer = (range_const_char*) &tmp_buffer.range_cast.const_cast, .wrote_size = &wrote_size)))
	    {
	    }

	    if (size < 0)
	    {
		log_fatal ("Could not write to output file descriptor");
	    }
	}

	buffer_rewrite (tmp_buffer);
    }

    if (size < 0)
    {
	log_fatal ("Could not read from input file descriptor");
    }

    if (!final_context (args.output, context, args.use_hashes))
    {
	log_fatal ("Could not finalize context");
    }
    
    free (tmp_buffer.begin);
    gcry_md_close (context);

    return true;

fail:
    if (context_initialized)
    {
	gcry_md_close (context);
    }
    free (tmp_buffer.begin);

    return false;
}

// identify name
metahash_id metahash_identify_name (const char * name)
{
    struct map { const char * name; metahash_id id; }
    *max, *i,
	map[] =
	{
	    { "sha256", METAHASH_SHA2_256 },
	    { "sha512", METAHASH_SHA2_512 },
	    { "md5", METAHASH_MD5_128 },
	};

    for (i = map, max = map + sizeof (map) / sizeof (map[0]); i < max; i++)
    {
	if (0 == strcmp (i->name, name))
	{
	    return i->id;
	}
    }

    return METAHASH_IDENTITY;
}

// identify list

bool metahash_identify_list (buffer_metahash_id * list, char * names)
{
    char * add = names;
    metahash_id id;

    while (*names)
    {
	if (*names == ',')
	{
	    *names = '\0';
	    id = metahash_identify_name (add);

	    if (!id)
	    {
		goto invalid_add;
	    }
	    
	    *buffer_push (*list) = id;
	    add = names + 1;
	}
	
	names++;
    }

    id = metahash_identify_name (add);

    if (!id)
    {
	goto invalid_add;
    }
    
    *buffer_push (*list) = id;

    return true;

invalid_add:
    
    log_fatal ("Invalid hash name '%s'", add);

fail:
    return false;
}
