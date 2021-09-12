#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#define FLAT_INCLUDES
#include "../keyargs/keyargs.h"
#include "pkg.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../tar/common.h"
#include "../tar/write.h"
#include "../log/log.h"
#include "../buffer_io/buffer_io.h"
#include "../dzip/dzip.h"
#include "../path/path.h"

#define FILE_BUFFER_SIZE (int)1e6

struct pkg_pack_state {
    int output_fd;
    buffer_unsigned_char tar_buffer;
    buffer_unsigned_char compress_buffer;
    dzip_deflate_state * dzip_deflate_state;
    size_t name_skip;
};

static void pkg_pack_state_free (pkg_pack_state * state)
{
    free (state->tar_buffer.begin);
    free (state->compress_buffer.begin);
    dzip_deflate_state_free(state->dzip_deflate_state);
    free (state);
}

keyargs_define(pkg_pack_new)
{
    pkg_pack_state * retval = calloc (1, sizeof(*retval));
    retval->output_fd = args.output_fd;
    retval->dzip_deflate_state = args.compression_type == PKG_PACK_COMPRESSION_DZIP ? dzip_deflate_state_new() : NULL;

    buffer_unsigned_char * script_buffer = &retval->compress_buffer; // a hack

    long int size;
    while (0 < (size = buffer_read (.buffer = &script_buffer->char_cast,
				    .fd = args.script_fd)))
    {}

    if (size < 0)
    {
	log_fatal ("Failed to read build script");
    }
    
    if (!tar_write_header (.output = &retval->tar_buffer.char_cast,
			   .name = "build.sh",
			   .mode = 0700,
			   .size = range_count (*script_buffer),
			   .type = TAR_FILE))
    {
	log_fatal ("Failed to add build script to tar");
    }

    log_debug ("read file: %s", script_buffer->begin);
    
    buffer_append(retval->tar_buffer, *script_buffer);

    tar_write_padding(&retval->tar_buffer.char_cast, range_count(*script_buffer));
    log_debug ("tar size %zu", range_count (retval->tar_buffer));

    buffer_rewrite(*script_buffer); // clean up after the hack
    
    return retval;

fail:
    pkg_pack_state_free (retval);
    return NULL;
}

static bool flush_buffer_to_fd (int fd, buffer_unsigned_char * buffer)
{
    if (range_is_empty(*buffer))
    {
	return true;
    }
    
    size_t wrote_size = 0;
    long int size;

    while (0 < (size = buffer_write (.fd = fd,
				     .buffer = &buffer->char_cast.range_cast.const_cast,
				     .wrote_size = &wrote_size)))
    {}

    if (size < 0)
    {
	log_fatal ("Failed to write buffer to output");
    }

    buffer_rewrite (*buffer);

    return true;

fail:
    return false;
}

static bool flush_tar_buffer (pkg_pack_state * state)
{
    if (state->dzip_deflate_state)
    {
	dzip_deflate(.state = state->dzip_deflate_state,
		     .input = &state->tar_buffer.range_cast.const_cast,
		     .output = &state->compress_buffer);
	buffer_rewrite(state->tar_buffer);
	log_debug ("used deflate");
	return true;
    }
    else
    {
	return flush_buffer_to_fd (state->output_fd, &state->tar_buffer);
    }
}

static bool flush_compress_buffer (pkg_pack_state * state)
{
    return flush_buffer_to_fd (state->output_fd, &state->compress_buffer);
}

static bool optional_flush_buffers (pkg_pack_state * state)
{
    if (range_count (state->tar_buffer) >= FILE_BUFFER_SIZE)
    {
	if (!flush_tar_buffer(state))
	{
	    log_fatal ("Failed to flush the tar buffer");
	}
    }

    if (range_count (state->compress_buffer) >= FILE_BUFFER_SIZE)
    {
	if (!flush_compress_buffer(state))
	{
	    log_fatal ("Failed to flush the compress buffer");
	}
    }

    return true;

fail:
    return false;
}

static bool pkg_pack_path_sub(pkg_pack_state * state, const char * path);
static bool pkg_pack_file(pkg_pack_state * state, const char * path, size_t size)
{
    int file_fd = open (path, O_RDONLY);

    if (file_fd < 0)
    {
	perror(path);
	log_fatal ("Could not open file %s", path);
    }

    size_t io_size;
    size_t start_size;
    
    optional_flush_buffers (state);

    while (true)
    {
	start_size = range_count(state->tar_buffer);
	
	while (0 < (io_size = buffer_read (.buffer = &state->tar_buffer.char_cast,
					   .initial_alloc_size = FILE_BUFFER_SIZE,
					   .max_buffer_size = FILE_BUFFER_SIZE,
					   .fd = file_fd)))
	{}

	if (io_size < 0)
	{
	    log_fatal ("Failed to read from file %s", path);
	}

	if (start_size == (size_t)range_count(state->tar_buffer))
	{
	    tar_write_padding(&state->tar_buffer.char_cast, size);
	    break;
	}

	optional_flush_buffers (state);
    }

    close (file_fd);
    
    return true;

fail:
    if (file_fd >= 0)
    {
	close(file_fd);
    }
    return false;
}


static bool pkg_pack_dir(pkg_pack_state * state, const char * path)
{
    DIR * dir = opendir (path);
    struct dirent * ent;

    buffer_char path_buffer = {0};
    
    while ( (ent = readdir (dir)) )
    {
	if (0 == strcmp (ent->d_name, "..") || 0 == strcmp (ent->d_name, "."))
	{
	    continue;
	}

	path_cat(&path_buffer, path, ent->d_name);
	//buffer_printf (&path_buffer, "%s%s%s", path, ends_with(path,PATH_SEPARATOR) ? "" : , ent->d_name);
	if (!pkg_pack_path_sub (state, path_buffer.begin))
	{
	    goto fail;
	}
    }

    free (path_buffer.begin);
    closedir (dir);
    return true;

fail:
    free (path_buffer.begin);
    closedir (dir);
    return false;
}

static bool pkg_pack_path_sub(pkg_pack_state * state, const char * path)
{
    unsigned long long size;
    tar_type type;

    assert (state->name_skip <= strlen(path));

    if (!tar_write_path_header(.output = &state->tar_buffer.char_cast,
			       .detect_type = &type,
			       .detect_size = &size,
			       .override_name = path + state->name_skip,
			       .path = path))
    {
	log_fatal ("Could not write tar header");
    }

    log_debug ("packing path %s", path);

    switch (type)
    {
    case TAR_FILE:    return pkg_pack_file(state, path, size);
    case TAR_DIR:     return pkg_pack_dir(state, path);
    case TAR_SYMLINK: return true;
    default: log_fatal("Invalid tar type for pkg file");
    }

    return true;

fail:
    return false;
}

bool pkg_pack_path(pkg_pack_state * state, const char * path)
{
    state->name_skip = strlen (path);
    return pkg_pack_path_sub (state, path);
}

bool pkg_pack_finish (pkg_pack_state * state)
{
    tar_write_end(&state->tar_buffer.char_cast);
    return flush_tar_buffer(state) && flush_compress_buffer(state);
}
