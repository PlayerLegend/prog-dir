#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "pkg.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "internal.h"
#include "../tar/common.h"
#include "../tar/read.h"
#include "../log/log.h"
#include "../path/path.h"
#include "../buffer_io/buffer_io.h"

static bool read_file_contents (buffer_char * output, tar_state * state, int fd)
{
    return tar_read_region(.buffer = output, .fd = fd, .size = state->file.size);
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

static bool write_file (const char * path, tar_state * state, int in_fd)
{
    assert (state->type == TAR_FILE);
    
    size_t buffer_size = 4 * TAR_BLOCK_SIZE;
    size_t written_size = 0;
    size_t chunk_size;
    int out_fd = open (path, O_WRONLY | O_CREAT, state->mode);
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

typedef struct {
    pkg_pack_compression_type compression_type;
    tar_state state;
}
    compressed_tar_state;

bool tar_update_compressed_fd (compressed_tar_state * state)
{
    return false;
}

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
	path_cat (&root->tmp.path, root->path.begin, state.path.begin);
	//buffer_printf (&root->tmp.path, "%s/%s", root->path.begin, state.path.begin);

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

