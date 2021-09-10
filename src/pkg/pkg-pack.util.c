#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "pkg.h"
#include "../log/log.h"

#define STDOUT_FILENO 1

void help_message(const char * program_name)
{
    log_stderr ("%s - create a package from one or more directories", program_name);
}

bool identify_compression_type (pkg_pack_compression_type * type, const char * name)
{
    if (0 == strcmp (name, "none"))
    {
	*type = PKG_PACK_COMPRESSION_NONE;
	return true;
    }
    else if (0 == strcmp (name, "dzip"))
    {
	*type = PKG_PACK_COMPRESSION_DZIP;
	return true;
    }
    else
    {
	log_fatal("Invalid compression type %s", name);
    }

fail:
    return false;
}

int main(int argc, char * argv[])
{
    pkg_pack_compression_type compression_type = PKG_PACK_COMPRESSION_DZIP;

    const char * script_path = NULL;

    static struct option long_options[] =
	{
	    {"help", no_argument, 0, 'h'},
	    {"compress", required_argument, 0, 'c'},
	    {"script", required_argument, 0, 's'},
	};

    int c;
    int option_index = 0;

    while ((c = getopt_long (argc, argv, "hc:s:", long_options, &option_index)))
    {
	if (c == -1)
	{
	    break;
	}

	switch (c)
	{
	case 'h':
	    help_message (argv[0]);
	    goto fail;

	case 'c':
	    if (!identify_compression_type(&compression_type, optarg))
	    {
		goto fail;
	    }
	    break;

	case 's':
	    if (script_path)
	    {
		log_fatal ("Multiple script arguments is invalid");
	    }

	    script_path = optarg;
	}
    }

    if (!script_path)
    {
	log_fatal ("No script path specified, use -s or --script");
    }

    int script_fd = open (script_path, O_RDONLY);

    if (script_fd < 0)
    {
	perror (script_path);
	log_fatal ("Could not open the script file %s", script_path);
    }
    
    pkg_pack_state * state = pkg_pack_new(.output_fd = STDOUT_FILENO,
					  .script_fd = script_fd,
					  .compression_type = compression_type);

    if (!state)
    {
	log_fatal ("Failed to create pkg-pack state");
    }

    for (int i = optind; i < argc; i++)
    {
	pkg_pack_path(state, argv[i]);
    }

    if (!pkg_pack_finish(state))
    {
	log_fatal ("Failed to finish package");
    }
    
    return 0;

fail:
    return 1;
}
