#include <stdbool.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#define FLAT_INCLUDES
#include "pkg.h"
#include "../log/log.h"

#define STDIN_FILENO 0

void help_message(const char * program_name)
{
    log_stderr ("%s - install package tar's", program_name);
}

int main(int argc, char * argv[])
{
    pkg_root * root = NULL;

    static struct option long_options[] =
	{
	    {"help", no_argument, 0, 'h'},
	    {"path", required_argument, 0, 'p'},
	};

    int c;
    int option_index = 0;

    while ((c = getopt_long (argc, argv, "hp:", long_options, &option_index)))
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

	case 'p':
	    if (root)
	    {
		log_fatal ("Multiple --path flags is invalid");
	    }
	    root = pkg_root_open(optarg);
	    break;
	}
    }

    if (!root)
    {
	root = pkg_root_open ("");
    }

    if (optind < argc)
    {
	log_fatal ("Installing packages by name is unimplemented");
    }
    else
    {
	log_debug ("installing from stdin");
	if (!pkg_install_fd (root, STDIN_FILENO))
	{
	    log_fatal ("Failed installing package from stdin");
	}
    }

    pkg_root_close(root);

    return 0;

fail:
    pkg_root_close(root);
    return 1;
}
