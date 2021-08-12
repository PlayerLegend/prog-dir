#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../vluint/vluint.h"
#include "metahash.h"
#include "../metabase/metabase.h"
#include "../buffer_io/buffer_io.h"
#include "../log/log.h"

#define fatal(mesg,...) { log_error(mesg,#__VA_ARGS__); goto fail; }

static struct {
    void (*encoder)(buffer_char*,const range_const_unsigned_char*);
    buffer_metahash_id hashes;
}
    program_state = { .encoder = metabase_encode };

static int open_name (const char * name)
{
    int fd = open (name, O_RDONLY);

    if (fd < 0)
    {
	if (0 == strcmp (name,"-"))
	{
	    fd = STDIN_FILENO;
	}
    }

    return fd;
}

static bool digest_file (const char * name)
{
    int fd = open_name (name);

    static buffer_unsigned_char digest_buffer;
    buffer_rewrite (digest_buffer);

    static buffer_char encode_buffer;
    buffer_rewrite (encode_buffer);

    if (!metahash_fd(.use_hashes = &program_state.hashes.range_cast.const_cast, .output = &digest_buffer, .in_fd = fd, .out_fd = -1))
    {
	return false;
    }
    
    program_state.encoder (&encode_buffer, (range_const_unsigned_char*) &digest_buffer);

    log_normal("%.*s  %s", (int)range_count(encode_buffer), encode_buffer.begin, name);
    
    close (fd);

    return true;
}

static void help_message(const char * program_name)
{
    log_error("usage: %s [flags] [files]", program_name);
    log_error("\n\tThis program generates metahash digests. Flags are:");
    log_error("\n--help: display this help message");
    log_error("-h,--hash [hash name]: specify the hash function to be used. Its argument can be: sha256,sha512,md5");
    log_error("-e,--encoding [encoding name]: specify the encoding to be used. Its argument can be: base16,base2");
    abort();
}

int main(int argc, char * argv[])
{
    if (argc == 1)
    {
	help_message(argv[0]);
    }
    
    int c;

    buffer_metahash_id use_hash_ids = {0};
    
    static struct option long_options[] =
	{
	    {"help", no_argument, 0, 256},
	    {"hash", required_argument, 0, 'h'},
	    {"encoding", required_argument, 0, 'e'},
	};

    int option_index = 0;

    while ((c = getopt_long (argc, argv, "h:e:", long_options, &option_index)))
    {
	if (c == -1)
	{
	    break;
	}
	
	switch (c)
	{
	case 256:
	    help_message(argv[0]);
	    
	case 'h':

	    if (!optarg)
	    {
		help_message (argv[0]);
	    }

	    if (!metahash_identify_list (&use_hash_ids, optarg))
	    {
		abort ();
	    }
	    
	    break;

	case 'e':
	    program_state.encoder = metabase_select_encoder(metabase_identify_name(optarg));
	    if (!program_state.encoder)
	    {
		abort ();
	    }
	    break;

	default:
	    abort();
	}
    }

    if (range_is_empty (program_state.hashes))
    {
	*buffer_push(program_state.hashes) = METAHASH_SHA2_256;
    }

    for (int i = optind; i < argc; i++)
    {
	if (!digest_file (argv[i]))
	{
	    abort();
	}
    }

    return 0;
}
