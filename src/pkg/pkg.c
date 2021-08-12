#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#define FLAT_INCLUDES
#include "pkg.h"
#include "../array/range.h"
#include "../array/buffer.h"
#include "../list/list.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"
#include "../log/log.h"
#include "../tar/tar.h"

#include "../table2/table.h"
#define table_string_value const char* package_name
#include "../table2/table-string.h"

struct pkg_root {
    buffer_char path;
    buffer_char logfile_path;
    table_string package_names;
    table_string path_to_package_name;
};

char * skip_isspace (char * input, bool pred)
{
    while (*input && pred == isspace(*input))
    {
	input++;
    }

    return input;
}

bool parse_log_line (char ** name, char ** path, char * line)
{
    *name = skip_isspace (line, true);
    *path = skip_isspace(skip_isspace(*name, false), true);

    return (*name != *path) && **path;
}

bool read_log (pkg_root * root, int fd)
{
    buffer_char read_buffer = {0};
    range_char line = {0};
    char * name;
    char * path;

    int line_number = 0;

    table_element * pair;

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

	table_include_element(&root->path_to_package_name, path)->child.package_name = table_include (&root->package_names, name);
    }

    return true;

fail:
    return false;
}

void pkg_root_close (pkg_root * root)
{
    table_clear(&root->package_names);
    table_clear(&root->path_to_package_name);
    free (root->path.begin);
    free (root->logfile_path.begin);
    free (root);
}

pkg_root * pkg_root_open(const char * path)
{
    pkg_root * retval = calloc (1, sizeof (*retval));
    int log_fd = -1;

    buffer_printf (&retval->path, "%s", path);
    buffer_printf (&retval->logfile_path, "%s/pkg/system.log");

    log_fd = open (retval->logfile_path.begin, O_RDONLY);

    if (0 > log_fd)
    {
	perror (retval->logfile_path.begin);
	log_fatal ("Could not open package log file");
    }

    if (!read_log (retval, log_fd))
    {
	log_fatal ("Failed to parse package log file");
    }

    close (log_fd);
    
    return retval;
    
fail:
    if (log_fd >= 0)
    {
	close (log_fd);
    }
    pkg_root_close (retval);
    return NULL;
}

bool pkg_install_name(const char * name);
bool pkg_install_fd(int fd)
{
    while (true)
    {
	//if (!tar_read_header
    }
}
