#define _XOPEN_SOURCE 500
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#define FLAT_INCLUDES
#include "../log/log.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"
#include "../base16/base16.h"
#include "../vluint/vluint.h"
#include "../metabase/metabase.h"
#include "../metahash/metahash.h"
#include "filestore.h"

bool filestore_path (buffer_char * output_path, const range_const_unsigned_char * input_digest)
{
    range_const_unsigned_char real_digest;
    metahash_id id;
    
    if (!metahash_unwrap (.id = &id, .digest = &real_digest, .input = input_digest))
    {
	return false;
    }

    assert (range_count (real_digest) > 1);
    
    buffer_char encode = {0};

    base16_encode (&encode, &real_digest);

    buffer_printf (output_path,
		   "%s/%02x/%02x/%.*s",
		   metahash_name (id),
		   real_digest.begin[0],
		   real_digest.begin[1],
		   (int)range_count (encode),
		   encode.begin);

    free (encode.begin);
    
    return true;
}

#define PATH_CHAR '/'

static bool mkdir_parents (const char * path)
{
    buffer_char parse = {0};
    buffer_printf (&parse, "%s", path);

    char * i = parse.begin;
    char swap;

    while (*i)
    {
	if (*i == PATH_CHAR )
	{
	    swap = i[1];
	    i[1] = '\0';
	    if (0 > mkdir (parse.begin, 0750) && errno != EEXIST)
	    {
		perror (parse.begin);
		goto fail;
	    }
	    i[1] = swap;
	}

	i++;
    }

    free (parse.begin);
    return true;

fail:
    free (parse.begin);
    return false;
}

keyargs_declare_static(bool, resolve_conflict,
		       const char * dst_path;
		       int (*link)(const char * oldpath, const char * newpath);
		       const range_const_metahash_id * use_hashes;
		       const range_const_unsigned_char * src_digest;
		       const char * src_path; bool replace;);
#define resolve_conflict(...) keyargs_call (resolve_conflict, __VA_ARGS__)

keyargs_define_static(resolve_conflict)
{
    int dst_fd = open (args.dst_path, O_RDONLY);
    buffer_unsigned_char dst_digest = {0};

    if (dst_fd < 0)
    {
	perror (args.dst_path);
	log_fatal ("Could not open destination path for conflict resolution");
    }

    if (0 == strcmp (args.src_path, args.dst_path))
    {
	goto success;
    }

    if (args.replace)
    {
	goto replace;
    }
    
    if (!metahash_fd (.in_fd = dst_fd, .out_fd = -1, .output = &dst_digest, .use_hashes = args.use_hashes))
    {
	log_error ("Could not digest conflicting destination file.");
	goto replace;
    }
    
    if (range_count (dst_digest) != range_count (*args.src_digest) || 0 != memcmp (dst_digest.begin, args.src_digest->begin, range_count (dst_digest)))
    {
	log_error ("File exists, but does not pass checksum.");

	goto replace;
    }

    goto success;
    
replace:
    log_stderr ("Replacing existing file '%s' with input file '%s'", args.dst_path, args.src_path);
    close (dst_fd);
    dst_fd = -1;
    unlink (args.dst_path);
    
    if (0 > args.link (args.src_path, args.dst_path))
    {
	perror (args.dst_path);
	log_fatal ("Cannot link %s -> %s", args.src_path, args.dst_path);
    }

success:
    
    if (dst_fd >= 1)
    {
	close (dst_fd);
    }
    free (dst_digest.begin);
    return true;

fail:
    if (dst_fd >= 1)
    {
	close (dst_fd);
    }
    free (dst_digest.begin);
    return false;
}

keyargs_declare_static(bool, link_digest,
		       int (*link)(const char * oldpath, const char * newpath);
		       const char * src_path;
		       const char * store_path;
		       const range_const_unsigned_char * src_digest;
		       const range_const_metahash_id * use_hashes;
		       bool replace;
		       bool existing_only;);

#define link_digest(...) keyargs_call(link_digest, __VA_ARGS__)

keyargs_define_static(link_digest)
{
    buffer_char relative_target_path = {0};
    buffer_char real_target_path = {0};

    if (!filestore_path (&relative_target_path, args.src_digest))
    {
	log_fatal ("Could not determine an output path");
    }

    buffer_printf (&real_target_path, "%s/%s", args.store_path, relative_target_path.begin);

    if (args.existing_only)
    {
	struct stat s;

	if (0 > stat (real_target_path.begin, &s))
	{
	    log_stderr ("Skipped %s", real_target_path.begin);
	    goto success;
	}
    }

    if (!mkdir_parents (real_target_path.begin))
    {
	goto fail;
    }
    
    if (0 > args.link (args.src_path, real_target_path.begin))
    {
	if (errno == EEXIST)
	{
	    if (!resolve_conflict(.dst_path = real_target_path.begin,
				  .link = args.link,
				  .use_hashes = args.use_hashes,
				  .src_path = args.src_path,
				  .src_digest = args.src_digest,
				  .replace = args.replace))
	    {
		goto fail;
	    }
	}
	else
	{
	    perror (real_target_path.begin);
	    log_fatal ("Could not create link %s -> %s", args.src_path, real_target_path.begin);
	}
    }

success:

    free (relative_target_path.begin);
    free (real_target_path.begin);

    return true;

fail:
    free (relative_target_path.begin);
    free (real_target_path.begin);

    return false;
}

keyargs_define (filestore_link)
{
    static char real_src_path[PATH_MAX + 1];

    if (!realpath (args.src_path, real_src_path))
    {
	perror (args.src_path);
	log_fatal ("Could not determine a realpath for %s", args.src_path);
    }
	    
    int src_fd = open (real_src_path, O_RDONLY);

    if (src_fd < 0)
    {
	perror (real_src_path);
	goto fail;
    }

    if (!metahash_fd (.in_fd = src_fd, .out_fd = -1, .output = args.result_digest, .use_hashes = args.use_hashes))
    {
	log_fatal ("Could not digest input");
    }

    if (!link_digest (.link = args.link,
		      .src_path = real_src_path,
		      .store_path = args.store_path,
		      .src_digest = &args.result_digest->range_cast.const_cast,
		      .use_hashes = args.use_hashes,
		      .replace = args.replace,
		      .existing_only = args.existing_only))
    {
	goto fail;
    }

    close (src_fd);

    return true;
    
fail:
    return false;
}

keyargs_define(filestore_copy)
{
    buffer_char real_tmp_path = {0};
    int tmp_fd = -1;

    buffer_printf (&real_tmp_path, "%s/tmp/copy.XXXXXX", args.store_path);

    if (!mkdir_parents (real_tmp_path.begin))
    {
	goto fail;
    }

    tmp_fd = mkstemp (real_tmp_path.begin);

    if (tmp_fd < 0)
    {
	perror ("mkstemp");
	log_fatal ("Could not create a temporary file '%s'", real_tmp_path.begin);
    }

    if (!metahash_fd (.in_fd = args.src_fd, .out_fd = tmp_fd, .output = args.result_digest, .use_hashes = args.use_hashes))
    {
	log_fatal ("Could not digest input");
    }

    if (!link_digest (.link = link,
		      .src_path = real_tmp_path.begin,
		      .store_path = args.store_path,
		      .src_digest = &args.result_digest->range_cast.const_cast,
		      .use_hashes = args.use_hashes,
		      .replace = args.replace,
		      .existing_only = args.existing_only))
    {
	goto fail;
    }
    
    close (tmp_fd);
    unlink (real_tmp_path.begin);
    free (real_tmp_path.begin);
    
    return true;

fail:
    if (tmp_fd >= 0)
    {
	close (tmp_fd);
	unlink (real_tmp_path.begin);
    }
    free (real_tmp_path.begin);
    return false;
}
