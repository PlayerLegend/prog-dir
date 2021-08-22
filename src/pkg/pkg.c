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

#define LOG_PATH "pkg/system.log"

struct pkg_root {
    struct {
	buffer_char name;
	buffer_char path;
	buffer_char build_sh;
    }
	tmp;
    
    buffer_char path;
    table_string log;
    buffer_const_string protected_paths;
};

// config functions

inline static bool file_exists(const char * path)
{
    struct stat s;
    return 0 == stat (path, &s);
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

static char * skip_isspace (char * input, bool pred)
{
    while (*input && pred == isspace(*input))
    {
	input++;
    }

    return input;
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

static void load_log_file_path(pkg_root * root)
{
    buffer_printf (&root->tmp.path, "%s/" LOG_PATH, root->path.begin);
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

static bool write_file (const char * path, tar_state * state, int in_fd)
{
    assert (state->type == TAR_FILE);
    
    size_t buffer_size = 4 * TAR_BLOCK_SIZE;
    size_t written_size = 0;
    size_t chunk_size;
    int out_fd = open(path, O_WRONLY | O_CREAT, state->mode);
    int write_result;
    size_t wrote_size = 0;

    if (out_fd < 0)
    {
	perror (path);
	log_fatal ("Could not open ouput file");
    }

    do {
	buffer_rewrite (state->tmp);

	chunk_size = state->file.size - written_size;
	if (chunk_size > buffer_size)
	{
	    chunk_size = buffer_size;
	}

	if (!tar_read_region(.buffer = &state->tmp,
			     .size = chunk_size,
			     .fd = in_fd))
	{
	    log_fatal ("Could not read tar file from file descriptor");
	}

	wrote_size = 0;
	while (0 < (write_result = buffer_write (.fd = out_fd,
						 .buffer = &state->tmp.range_cast.const_cast,
						 .wrote_size = &wrote_size)))
	{}

	if (write_result < 0)
	{
	    perror (path);
	    log_fatal ("Could not write to output file");
	}

	written_size += chunk_size;
    }
    while (written_size < state->file.size);

    return true;
    
fail:
    return false;
}

static bool take_ownership (pkg_root * root)
{
    table_string_item * path_item = table_string_include (root->log, root->tmp.path.begin);

    if (path_item->value.package_name && 0 != strcmp(path_item->value.package_name, root->tmp.name.begin))
    {
	log_fatal ("Path %s is owned by %s, cannot allocate for %s", root->tmp.path.begin, path_item->value.package_name, root->tmp.name.begin);
    }
    else
    {
	path_item->value.package_name = table_string_include (root->log, root->tmp.name.begin)->query.key;
	return true;
    }

fail:
    return false;
}

static bool verify_build_sh_name (tar_state * state)
{
    if (state->type != TAR_FILE)
    {
	log_fatal ("Expected a file for build.sh");
    }

    const char * basename = strchr (state->path.begin, PATH_SEPARATOR);
    
    if (!basename)
    {
	basename = state->path.begin;
    }
    else
    {
	basename++;
    }

    if (0 != strcmp (basename, "build.sh"))
    {
	log_fatal ("First file is not named build.sh -- the package info file");
    }

    return true;

fail:
    return false;
}

static bool read_file_contents (buffer_char * output, tar_state * state, int fd)
{
    return tar_read_region(.buffer = output, .fd = fd, .size = state->file.size);
}

static bool parse_build_sh (pkg_root * root, const char * contents)
{
    const char * name_line = strstr (contents, "PKG_NAME=");

    if (!name_line)
    {
	log_fatal ("Could not determine package name from build.sh by searching for PKG_NAME=");
    }

    const char * name = name_line + 9; //length of PKG_NAME=
    char * end = strchr (name, '\n');

    if (end)
    {
	*end = '\0';
    }
    
    if (*name == '"' || *name == '\'')
    {
	name++;
	end = strchr (name, '"');
	if (end)
	{
	    *end = '\0';
	}
	end = strchr (name, '\'');
	if (end)
	{
	    *end = '\0';
	}
    }

    const char * i = name;

    while (*i)
    {
	if (isspace(*i))
	{
	    log_fatal ("Whitespace in package name");
	}

	i++;
    }

    buffer_strcpy (&root->tmp.name, name);

    return true;
    
fail:
    return false;
}

static bool is_protected_compare (const char * check, const char * arg)
{
    while (*check && *check == *arg)
    {
	check++;
	arg++;
    }

    if (check[0] == arg[0])
    {
	return true;
    }

    if ((arg[0] == '\0' || (arg[0] == PATH_SEPARATOR && arg[1] == '\0'))
	&& check[0] == PATH_SEPARATOR)
    {
	return true;
    }
    
    return false;
}

static bool is_protected(pkg_root * root, const char * check)
{
    const char ** protect_arg;
    
    for_range (protect_arg, root->protected_paths)
    {
        if (is_protected_compare (check, *protect_arg))
	{
	    return true;
	}
    }

    return false;
}

bool pkg_install_name(pkg_root * root, const char * name);

bool pkg_install_fd(pkg_root * root, int fd)
{
    tar_state state = {0};
    
    while (tar_update_fd (&state, fd) && state.type != TAR_FILE)
    {
	if (state.type != TAR_DIR)
	{
	    log_fatal ("Expected only directories preceeding build.sh in this tar");
	}
    }

    if (!verify_build_sh_name(&state))
    {
	log_fatal ("Invalid file for build.sh");
    }

    if (!read_file_contents(&root->tmp.build_sh, &state, fd))
    {
	log_fatal ("Could not read file for build.sh");
    }

    if (!parse_build_sh(root, root->tmp.build_sh.begin))
    {
	log_fatal ("Failed to parse build.sh");
    }

    while (tar_update_fd(&state, fd))
    {
	buffer_printf (&root->tmp.path, "%s/%s", root->path.begin, state.path.begin);

	log_normal ("Installing %s: %s", root->tmp.name.begin, root->tmp.path.begin);
	
	switch (state.type)
	{
	case TAR_DIR:
	    
	    if (!path_mkdir_target(root->tmp.path.begin))
	    {
		log_fatal ("Could not create a package path");
	    }
	    break;

	case TAR_FILE:

	    if (file_exists (root->tmp.path.begin) && is_protected (root, state.path.begin))
	    {
		log_normal ("File %s exists and is protected", root->tmp.path.begin);
		tar_skip_file(&state, fd);
		break;
	    }
	    
	    if (!path_mkdir_parents(root->tmp.path.begin))
	    {
		log_fatal ("Could not create a package path");
	    }

	    if (!take_ownership (root))
	    {
		log_fatal ("Cannot install file %s", root->tmp.path.begin);
	    }
	    
	    if (!write_file (root->tmp.path.begin, &state, fd))
	    {
		log_fatal ("Could not write file %s", root->tmp.path.begin);
	    }
	    break;

	default:
	    continue;
	}
    }

    tar_cleanup (&state);
    
    return true;

fail:
    tar_cleanup (&state);
    return false;
}

