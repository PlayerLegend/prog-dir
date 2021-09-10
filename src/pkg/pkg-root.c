#define _XOPEN_SOURCE 700 // for dprintf
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "pkg.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "internal.h"
#include "../buffer_io/buffer_io.h"
#include "../path/path.h"
#include "../log/log.h"
#include "../immutable/immutable.h"
#include "../paren-parser/paren-parser.h"
#include "../paren-parser/paren-preprocessor.h"

static void load_log_file_path(pkg_root * root)
{
    buffer_printf (&root->tmp.path, "%s/" LOG_PATH, root->path.begin);
}

static char * skip_isspace (char * input, bool pred)
{
    while (*input && pred == isspace(*input))
    {
	input++;
    }

    return input;
}

static bool dump_log_to_fd(int fd, table_string * log)
{
    table_string_bucket bucket;
    table_string_item * item;
    for_table(bucket, *log)
    {
	for_table_bucket(item, bucket)
	{
	    if (item->value.package_name)
	    {
		if (0 > dprintf (fd, "%s %s\n", item->value.package_name, item->query.key))
		{
		    perror ("dump_log_to_fd");
		    return false;
		}
	    }
	}
    }

    return true;
}

static bool dump_log(pkg_root * root)
{
    int log_fd = -1;

    load_log_file_path (root);
    
    if (!path_mkdir_parents(root->tmp.path.begin))
    {
	log_fatal ("Could not create parent directories the package log");
    }
    
    log_fd = open (root->tmp.path.begin, O_CREAT | O_TRUNC | O_WRONLY, 0644);

    if (log_fd < 0)
    {
	log_fatal ("Could not open the log file at %s", root->tmp.path.begin);
    }

    if (!dump_log_to_fd (log_fd, &root->log))
    {
	log_fatal ("Could not write to log file");
    }

    assert (log_fd >= 0);
    close (log_fd);
    
    return true;
    
fail:

    if (log_fd >= 0)
    {
	close (log_fd);
    }
    
    return false;
}

static bool paren_config_entry_protected_paths (pkg_root * root, paren_atom * key)
{
    paren_atom * i = key->peer;

    while (i)
    {
	if (!i->child.is_text)
	{
	    paren_fatal (i, "Arguments to %s should be text", key->child.text);
	}

	*buffer_push(root->protected_paths) = table_string_include(root->log, i->child.text)->query.key;
	
	i = i->peer;
    }

    return true;

fail:
    return false;
}

static bool parse_config_entry (pkg_root * root, paren_atom * atom)
{
    if (atom->child.is_text)
    {
	paren_fatal (atom, "Text node at root of config, this should be in parentheses");
    }

    paren_atom * key = atom->child.atom;

    if (!key->child.is_text)
    {
	paren_fatal (key, "Expected a text key, e.g. repos or protected-paths");
    }

    if (0 == strcmp (key->child.text, "protect-paths"))
    {
	if (!paren_config_entry_protected_paths (root, key))
	{
	    paren_fatal (atom, "Could not parse a protected-paths item");
	}
    }
    else
    {
	paren_warning (atom, "invalid config key '%s'", key->child.text);
    }

    return true;

fail:
    return false;
}

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

static bool parse_log_line (char ** name, char ** path, char * line)
{
    *name = skip_isspace (line, true);
    *path = skip_isspace(*name, false);

    if (!*path)
    {
	return false;
    }
    
    **path = '\0';
    
    *path = skip_isspace(*path + 1, true);

    return (*name != *path) && **path;
}

static bool read_log_fd (pkg_root * root, int fd)
{
    buffer_char read_buffer = {0};
    range_char line = {0};
    char * name;
    char * path;

    int line_number = 0;
    
    while (buffer_getline_fd (.line = &line,
			      .read.fd = fd,
			      .read.buffer = &read_buffer))
    {
	line_number++;
	*line.end = '\0';

	if (!parse_log_line (&name, &path, line.begin))
	{
	    log_fatal ("Failed to parse log line %d", line_number);
	}

	table_string_include(root->log, path)->value.package_name = table_string_include (root->log, name)->query.key;
    }

    free (read_buffer.begin);

    return true;

fail:
    free (read_buffer.begin);
    return false;
}

static bool read_log (pkg_root * root)
{
    int log_fd = -1;
    
    load_log_file_path (root);
    
    log_fd = open (root->tmp.path.begin, O_RDONLY);

    if (0 > log_fd)
    {
	if (errno == ENOENT)
	{
	    return true;
	}
	
	perror (root->path.begin);
	log_fatal ("Could not open package log file");
    }

    if (!read_log_fd (root, log_fd))
    {
	log_fatal ("Failed to parse package log file");
    }

    assert (log_fd >= 0);
    close (log_fd);

    return true;

fail:
    if (log_fd >= 0)
    {
	close (log_fd);
    }
    return false;
}

pkg_root * pkg_root_open(const char * path)
{
    pkg_root * retval = calloc (1, sizeof (*retval));

    buffer_printf (&retval->path, "%s", path);

    if (!load_config (retval))
    {
	log_fatal ("Failed to load config");
    }

    if (!read_log (retval))
    {
	log_fatal ("Failed to read package log file");
    }

    return retval;
    
fail:
    pkg_root_close (retval);
    return NULL;
}

void pkg_root_close (pkg_root * root)
{
    dump_log (root);
    
    table_string_clear(root->log);
    free (root->path.begin);
    free (root->tmp.path.begin);
    free (root->tmp.name.begin);
    free (root->tmp.build_sh.begin);
    free (root->protected_paths.begin);
    free (root);
}
