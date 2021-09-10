#define _XOPEN_SOURCE 700 // for dprintf
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#define FLAT_INCLUDES
#include "pkg.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../list/list.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"
#include "../log/log.h"
#include "../tar/tar.h"
#include "../path/path.h"
#include "../immutable/immutable.h"
#include "../paren-parser/paren-parser.h"
#include "../paren-parser/paren-preprocessor.h"

#include "../table2/table.h"
#define table_string_value const char* package_name;
#include "../table2/table-string.h"


// config functions


static bool load_config (pkg_root * root)
{
    buffer_printf (&root->tmp.path, "%s/etc/pkg/config", root->path.begin);
    const char * config_path = root->tmp.path.begin;

    if (!file_exists(config_path))
    {
	return true;
    }
    
    paren_atom * root_atom = paren_preprocessor(.filename = config_path);

    if (!root_atom)
    {
	log_fatal ("Failed to load config %s", config_path);
    }

    paren_atom * i = root_atom;

    do {
	if (!parse_config_entry(root, i))
	{
	    log_fatal ("Could not parse a config entry");
	}
    }
    while ( (i = i->peer) );

    paren_atom_free (root_atom);
    
    return true;

fail:
    paren_atom_free (root_atom);
    return false;
}

bool pkg_install_name(pkg_root * root, const char * name);
