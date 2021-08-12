#define _XOPEN_SOURCE 500 // for symlink
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <unistd.h>
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

static struct {
    buffer_metahash_id use_hashes;
    const char * store_path;
    void (*encoder)(buffer_char*,const range_const_unsigned_char*);
    enum { STORE_COPY, STORE_HARDLINK, STORE_SYMLINK } store_type;
    bool replace;
    bool existing_only;
    bool stdin_paths;
} program_state = { .encoder = metabase_encode_base16 };

bool copy_fd (int fd)
{
    assert (!range_is_empty (program_state.use_hashes));
    assert (program_state.store_path);
    
    static buffer_unsigned_char digest = {0};
    buffer_rewrite (digest);
    
    if (!filestore_copy (.result_digest = &digest,
			 .use_hashes = &program_state.use_hashes.range_cast.const_cast,
			 .store_path = program_state.store_path,
			 .src_fd = fd,
			 .replace = program_state.replace,
			 .existing_only = program_state.existing_only))
    {
	log_fatal ("Failed to copy file into %s", program_state.store_path);
    }

    static buffer_char digest_armor = {0};
    buffer_rewrite (digest_armor);

    metabase_encode_base16(&digest_armor, &digest.range_cast.const_cast);

    log_normal ("%.*s", (int)range_count(digest_armor), digest_armor.begin);

    return true;
    
fail:
    return false;
}

bool link_path (const char * src_path)
{
    assert (!range_is_empty (program_state.use_hashes));
    assert (program_state.store_path);

    static buffer_unsigned_char digest = {0};
    buffer_rewrite (digest);

    if (!filestore_link (.result_digest = &digest,
			 .link = program_state.store_type == STORE_SYMLINK ? symlink : link,
			 .use_hashes = &program_state.use_hashes.range_cast.const_cast,
			 .store_path = program_state.store_path,
			 .src_path = src_path,
			 .replace = program_state.replace,
			 .existing_only = program_state.existing_only))
    {
	log_fatal ("Failed to link file into %s", program_state.store_path);
    }

    static buffer_char digest_armor = {0};
    buffer_rewrite (digest_armor);

    metabase_encode_base16(&digest_armor, &digest.range_cast.const_cast);

    log_normal ("%.*s", (int)range_count(digest_armor), digest_armor.begin);

    return true;
    
fail:
    return false;
    
}

bool store_path (const char * src_path)
{
    int src_fd = -1;
    
    switch (program_state.store_type)
    {
    case STORE_COPY:
	src_fd = open (src_path, O_RDONLY);

	if (src_fd < 0)
	{
	    perror (src_path);
	    goto fail;
	}

	if (!copy_fd (src_fd))
	{
	    log_fatal ("Could not copy %s into store", src_path);
	}

	close (src_fd);
	src_fd = -1;
	return true;

    case STORE_HARDLINK:
    case STORE_SYMLINK:
	if (!link_path (src_path))
	{
	    goto fail;
	}
	return true;
    }

fail:

    if (src_fd >= 0)
    {
	close (src_fd);
    }

    return false;
}

static void help_message(const char * program_name)
{
    log_error("usage: %s [flags] [files]", program_name);
    log_error("\n\tThis program copies files into a filestore directory. Flags are:");
    log_error("\n--help: display this help message");
    log_error("-h,--hash [hash name]: specify the hash function to be used. Its argument can be: sha256,sha512,md5");
    log_error("-e,--encoding [encoding name]: specify the encoding to be used. Its argument can be: base16,base2");
    log_error("-p,--path [path]: specify the filestore path");
    abort();
}

enum {
    ARG_HASH = 'h',
    ARG_ENCODING = 'e',
    ARG_PATH = 'p',
    ARG_TYPE = 't',
    
    ARG_HELP = 256,
    ARG_REPLACE,
    ARG_EXISTING_ONLY,
    ARG_STDIN_PATHS,
};

int main(int argc, char *argv[])
{
    int c;

    buffer_metahash_id use_hash_ids = {0};

    static struct option long_options[] =
	{
	    {"help", no_argument, 0, ARG_HELP},
	    {"hash", required_argument, 0, ARG_HASH},
	    {"encoding", required_argument, 0, ARG_ENCODING},
	    {"path", required_argument, 0, ARG_PATH},
	    {"type", required_argument, 0, ARG_TYPE},
	    {"replace", no_argument, 0, ARG_REPLACE},
	    {"existing-only", no_argument, 0, ARG_EXISTING_ONLY},
	    {"stdin-paths", no_argument, 0, ARG_STDIN_PATHS},
	    {0},
	};

    int option_index = 0;

    while ((c = getopt_long (argc, argv, "h:e:p:t:", long_options, &option_index)))
    {
	if (c == -1)
	{
	    break;
	}
	
	switch (c)
	{
	case ARG_HELP:
	    help_message(argv[0]);

	case ARG_HASH:

	    if (!optarg)
	    {
		help_message (argv[0]);
	    }

	    if (!metahash_identify_list (&use_hash_ids, optarg))
	    {
		log_fatal ("Bad argument to -h,--hash");
	    }
	    
	    break;

	case ARG_ENCODING:

	    program_state.encoder = metabase_select_encoder(metabase_identify_name(optarg));
	    if (!program_state.encoder)
	    {
		log_fatal ("Bad argument to -e,--encoder");
	    }
	    
	    break;

	case ARG_PATH:

	    if (!optarg)
	    {
		log_fatal ("-p,--path must specify a storage path");
	    }
	    
	    program_state.store_path = optarg;

	    break;

	case ARG_TYPE:

	    if (!optarg)
	    {
		log_fatal ("-t,--type must specify a storage type");
	    }

	    if (0 == strcmp (optarg, "copy"))
	    {
		program_state.store_type = STORE_COPY;
	    }
	    else if (0 == strcmp (optarg, "hardlink"))
	    {
		program_state.store_type = STORE_HARDLINK;
	    }
	    else if (0 == strcmp (optarg, "symlink"))
	    {
		program_state.store_type = STORE_SYMLINK;
	    }
	    else
	    {
		log_fatal ("-t,--type: Invalid argument '%s', must be one of: copy, hardlink, symlink", optarg);
	    }
	    break;

	case ARG_REPLACE:
	    program_state.replace = true;
	    break;

	case ARG_EXISTING_ONLY:
	    program_state.existing_only = true;
	    break;

	case ARG_STDIN_PATHS:
	    program_state.stdin_paths = true;
	    break;

	default:
	    goto fail;
	    //log_fatal ("Bad value in args switch: %c(%d)", c, c);
	}
    }

    if (range_is_empty (program_state.use_hashes))
    {
	*buffer_push(program_state.use_hashes) = METAHASH_SHA2_256;
    }

    if (program_state.stdin_paths)
    {
	range_char line;
	buffer_char line_buffer = {0};

	while (buffer_getline_fd (.read.fd = STDIN_FILENO, .read.buffer = &line_buffer, .line = &line))
	{
	    *line.end = '\0';

	    if (!store_path (line.begin))
	    {
		goto fail;
	    }
	}
    }
    else if (optind < argc)
    {
        for (int i = optind; i < argc; i++)
	{
	    if (!store_path(argv[i]))
	    {
		goto fail;
	    }
	}
    }
    else
    {
	if (program_state.store_type != STORE_COPY)
	{
	    log_fatal ("Cannot link from stdin");
	}
	
	if (!copy_fd (STDIN_FILENO))
	{
	    goto fail;
	}
    }

    return 0;

fail:
    return 1;
}
