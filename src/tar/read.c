#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"
#include "common.h"
#include "read.h"
#include "../log/log.h"
#include "internal/spec.h"

void tar_restart(tar_state * state)
{
    state->type = TAR_ERROR;
    buffer_rewrite (state->path);
    buffer_rewrite (state->tmp);
    buffer_rewrite (state->link.path);
}

static bool tar_get_size (size_t * size, const struct posix_header * header)
{
    char * endptr;

    if (header->size[0] & 128) // base 256 encoding
    {
	log_error ("base 256 not implemented");
	return false;
    }
    else // null terminated octal encoding
    {
	*size = strtol (header->size, &endptr, 8);
	if (!*header->size || *endptr)
	{
	    log_error ("could not parse size");
	    return false;
	}
    }

    return true;
}

static bool tar_get_mode (size_t * mode, const struct posix_header * header)
{
    char * endptr;
    
    *mode = strtol (header->mode, &endptr, 8);
    if (!*header->mode || *endptr)
    {
	log_error ("could not parse mode, error at byte %zu", endptr - header->mode);
	return false;
    }

    return true;
}

static bool block_is_zero (const range_const_char * header)
{
    for (uint64_t *test = (void*) header->begin; (void*) test < (void*) header->end; test++)
    {
	if (*test != 0)
	{
	    return false;
	}
    }
    
    return true;
}

bool record_mem (buffer_char * buffer, range_const_char * mem, long long int size)
{
    long long int want_size = (size % TAR_BLOCK_SIZE == 0) ? size : (((size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE);
    
    if (range_count (*mem) < want_size)
    {
	return false;
    }

    size_t old_size = range_count(*buffer);
    size_t new_size = old_size + want_size + 1;

    buffer_resize (*buffer, new_size);
    buffer->end--;
    *buffer->end = '\0';
    memcpy (buffer->begin + old_size, mem->begin, want_size);
    assert (*buffer->end == '\0');
    mem->begin += want_size;

    return true;
}

bool tar_update_mem (tar_state * state, range_const_char * rest, const range_const_char * mem)
{
    state->ready = false;

    bool longname = false;
    bool longlink = false;

    range_const_char rest_substitute;

    if (!rest)
    {
	rest = &rest_substitute;
    }

    *rest = *mem;
    
    if (state->type == TAR_LONGNAME)
    {
	if ((size_t) range_count(state->path) < state->file.size)
	{
	    record_mem (&state->path, rest, state->file.size - range_count (state->path));
	    goto notready;
	}

	if ((size_t)range_count(state->path) != state->file.size)
	{
	    log_fatal ("tar longname path is oversized");
	}

	longname = true;
    }
    else if (state->type == TAR_LONGLINK)
    {
	if ((size_t) range_count(state->link.path) < state->file.size)
	{
	    record_mem (&state->link.path, rest, state->file.size - range_count (state->link.path));
	    goto notready;
	}
	
	if ((size_t)range_count(state->link.path) != state->file.size)
	{
	    log_fatal ("tar longlink path is oversized");
	}

	longlink = true;
    }

    if (range_count (*mem) < TAR_BLOCK_SIZE)
    {
	goto notready;
    }
    
    range_const_char header_mem = { .begin = mem->begin, .end = mem->begin + TAR_BLOCK_SIZE };
    rest->begin += TAR_BLOCK_SIZE;

    const struct posix_header * header = (void*) header_mem.begin;
    assert (sizeof(*header) <= (size_t)range_count (header_mem));
    
    if (block_is_zero (&header_mem))
    {
	if (state->type == TAR_END)
	{
	    goto done;
	}
	else
	{
	    state->type = TAR_END;
	    goto notready;
	}
    }
    else if(header->typeflag == REGTYPE)
    {
	state->type = TAR_FILE;
    }
    else if (header->typeflag == DIRTYPE)
    {
	state->type = TAR_DIR;
    }
    else if (header->typeflag == SYMTYPE)
    {
	state->type = TAR_SYMLINK;
    }
    else if (header->typeflag == LNKTYPE)
    {
	state->type = LNKTYPE;
    }
    else
    {
	if (header->typeflag == GNUTYPE_LONGNAME)
	{
	    state->type = TAR_LONGNAME;
	    buffer_rewrite (state->path);
	}
	else if (header->typeflag == GNUTYPE_LONGLINK)
	{
	    state->type = TAR_LONGLINK;
	    buffer_rewrite (state->link.path);
	}
	else
	{
	    log_fatal ("Invalid typeflag in tar header");
	}

	goto notready;
    }
	
    if (!longname)
    {
	buffer_printf (&state->path, "%s", header->name);
    }
    
    if (state->type == TAR_HARDLINK || state->type == TAR_SYMLINK)
    {
	if (!longlink)
	{
	    buffer_printf (&state->link.path, "%s", header->linkname);
	}
    }
    else if (longlink)
    {
	log_fatal ("Longlink specified for non-link type (%d)", state->type);
    }
    
    if (state->type == TAR_FILE)
    {
	if (!tar_get_size (&state->file.size, header))
	{
	    log_fatal ("Could not read tar file size");
	}
    }

    if (state->type == TAR_FILE || state->type == TAR_DIR || state->type == TAR_HARDLINK || state->type == TAR_SYMLINK)
    {
	if (!tar_get_mode (&state->mode, header))
	{
	    log_fatal ("Could not read tar file mode");
	}
    }
    
//ready:
    state->ready = true;
    return true;

fail:
    state->type = TAR_ERROR;
    state->ready = true;
    return false;
    
notready:
    state->ready = false;
    return true;

done:
    state->type = TAR_END;
    state->ready = true;
    return false;
}

bool tar_update_fd (tar_state * state, int fd)
{
    long int size;
    
    while (true)
    {
	buffer_rewrite (state->tmp);
	
	while (0 < (size = buffer_read (.fd = fd, .buffer = &state->tmp, .max_buffer_size = TAR_BLOCK_SIZE)))
	{}

	if (size < 0)
	{
	    log_fatal ("Error reading from file descriptor");
	}

	if (range_count (state->tmp) < TAR_BLOCK_SIZE)
	{
	    log_fatal ("Input closed prematurely");
	}

	if (!tar_update_mem (state, NULL, &state->tmp.range_cast.const_cast))
	{
	    goto done;
	}

	if (state->ready)
	{
	    goto newcontents;
	}
    }

fail:
    state->type = TAR_ERROR;
    state->ready = true;
    return false;

newcontents:
    return true;
    
done:
    return false;
}

bool tar_skip_file (tar_state * state, int fd)
{
    assert (state->type == TAR_FILE);
    
    size_t skip_size = (state->file.size % TAR_BLOCK_SIZE == 0) ? state->file.size : (((state->file.size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE);

    assert (skip_size % TAR_BLOCK_SIZE == 0);

    while (skip_size > 0)
    {
	buffer_rewrite (state->tmp);
	if (!tar_read_region (.fd = fd, .buffer = &state->tmp, .size = TAR_BLOCK_SIZE))
	{
	    log_fatal ("Could not read from fd while skipping tar file");
	}
	skip_size -= TAR_BLOCK_SIZE;
    }

    return true;

fail:
    return false;
}

keyargs_define(tar_read_region)
{
    if (!args.size)
    {
	return true;
    }
    
    size_t max_buffer_size = (args.size % TAR_BLOCK_SIZE == 0) ? args.size : (((args.size / TAR_BLOCK_SIZE) + 1) * TAR_BLOCK_SIZE);

    long int size;
    
    while (0 < (size = buffer_read (.fd = args.fd,
				    .buffer = args.buffer,
				    .max_buffer_size = max_buffer_size)))
    {}

    if (size < 0)
    {
	log_fatal ("Failed reading tar");
    }

    if ((size_t)range_count (*args.buffer) != max_buffer_size)
    {
	log_fatal ("Tar input ended during block read");
    }

    buffer_resize (*args.buffer, args.size);

    return true;

fail:
    return false;
}

void tar_cleanup (tar_state * state)
{
    free (state->path.begin);
    buffer_rewrite (state->path);
    free (state->tmp.begin);
    buffer_rewrite (state->tmp);
    free (state->link.path.begin);
    buffer_rewrite (state->link.path);
}
