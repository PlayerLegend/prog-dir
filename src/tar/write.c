#define _XOPEN_SOURCE 500
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>
#include <limits.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "../buffer_io/buffer_io.h"
#include "common.h"
#include "write.h"
#include "internal/spec.h"
#include "../log/log.h"
#include "../path/path.h"

inline static bool convert_type(char * output, tar_type input)
{
    switch (input)
    {
    case TAR_DIR:
	*output = DIRTYPE;
	return true;

    case TAR_FILE:
	*output = REGTYPE;
	return true;

    case TAR_HARDLINK:
	*output = LNKTYPE;
	return true;

    case TAR_SYMLINK:
	*output = SYMTYPE;
	return true;

    case TAR_LONGLINK:
	*output = GNUTYPE_LONGLINK;
	return true;

    case TAR_LONGNAME:
	*output = GNUTYPE_LONGNAME;
	return true;

    default:
	log_fatal ("Invalid tar item type");
    }

fail:
    return false;
}

static bool ends_with (const char * string, char c)
{
    if (!*string)
    {
	return false;
    }

    while (string[1])
    {
	string++;
    }

    return *string == c;
}

keyargs_define(tar_write_header)
{
    union {
	struct posix_header posix;
	char bytes[TAR_BLOCK_SIZE];
    }
    header;

    memset (&header.posix, 0, sizeof(header.posix));
    //memset (&header.posix, 0, sizeof(header.posix));
    //memset (header.bytes + sizeof(header.posix), ' ', sizeof(header) - sizeof(header.posix));
    

    struct passwd * passwd = args.uname ? getpwnam (args.uname) : getpwuid(args.uid);

    if (!passwd)
    {
	log_fatal ("Failed to lookup passwd entry for tar file's user");
    }
    
    struct group * group = args.gname ? getgrnam(args.gname) : getgrgid(args.gid);

    if (!group)
    {
	log_fatal ("Failed to lookup group entry for tar file's group");
    }

    assert (args.name);

    while (*args.name == PATH_SEPARATOR)
    {
	args.name++;
    }

    unsigned long long size = strlen (args.name) + 1;
    bool add_sep = args.type == TAR_DIR && !ends_with (args.name, PATH_SEPARATOR);

    if (size + (add_sep ? 1 : 0) < sizeof(header.posix.name))
    {
	memset (header.posix.name, 0, sizeof(header.posix.name));
	strcpy (header.posix.name, args.name);

	if (add_sep)
	{
	    strcat (header.posix.name, (char[]){ PATH_SEPARATOR, '\0' });
	}
    }
    else
    {
	tar_write_header (.output = args.output,
			  .name = "././@LongName",
			  .size = size,
			  .type = TAR_LONGNAME);

	buffer_append_n(*args.output, args.name, size);

	if (add_sep)
	{
	    args.output->end--;
	    *buffer_push (*args.output) = PATH_SEPARATOR;
	    *buffer_push (*args.output) = '\0';
	}
	
	tar_write_padding(args.output, size);
    }

    snprintf (header.posix.mode, sizeof(header.posix.mode), "%07o", args.mode);
    snprintf (header.posix.uid, sizeof(header.posix.uid), "%07o", passwd->pw_uid);
    snprintf (header.posix.gid, sizeof(header.posix.gid), "%07o", group->gr_gid);

    if (args.size <= 077777777777)
    {
	snprintf (header.posix.size, sizeof(header.posix.size), "%011llo", args.size);
    }
    else
    {
	log_fatal ("Tar size exceeds maximum for octal sizes, base 256 is unimplemented");
    }

    snprintf (header.posix.mtime, sizeof(header.posix.mtime), "%011llo", args.mtime);

    if (!convert_type (&header.posix.typeflag, args.type))
    {
	log_fatal ("Could not convert given tar entry type");
    }

    if (args.linkname)
    {
	size = strlen (args.linkname) + 1;

	if (size < sizeof(header.posix.linkname))
	{
	    strcpy (header.posix.linkname, args.linkname);
	}
	else
	{
	    tar_write_header (.output = args.output,
			      .name = "././@LongLink",
			      .size = size,
			      .type = TAR_LONGLINK);
	    buffer_append_n (*args.output, args.name, size);
	    tar_write_padding(args.output, size);
	}
    }
    else
    {
	memset (header.posix.linkname, 0, sizeof(header.posix.linkname));
    }

    assert (sizeof(header.posix.magic) == 6);
    memcpy(header.posix.magic, (char[]){ 'u', 's', 't', 'a', 'r', ' ' }, 6);

    assert (sizeof(header.posix.version) == 2);
    memcpy(header.posix.version, (char[]){ ' ', 0 }, 2);

    strncpy(header.posix.uname, passwd->pw_name, sizeof(header.posix.uname));

    strncpy(header.posix.gname, group->gr_name, sizeof(header.posix.gname));

    memset (header.posix.devmajor, 0, sizeof(header.posix.devmajor));
    memset (header.posix.devminor, 0, sizeof(header.posix.devminor));
    memset (header.posix.chksum, ' ', sizeof(header.posix.chksum));
    memset (header.posix.prefix, 0, sizeof(header.posix.prefix));
    
    unsigned int checksum = 0;

    for (unsigned int i = 0; i < sizeof(header); i++)
    {
	checksum += header.bytes[i];
    }

    snprintf (header.posix.chksum, sizeof(header.posix.chksum), "%07o", checksum);

    buffer_append_n(*args.output, (char*)&header, sizeof(header));

    return true;
    
fail:
    return false;
}

void tar_write_padding (buffer_char * output, unsigned long long file_size)
{
    size_t block_have = file_size % TAR_BLOCK_SIZE;

    if (!block_have)
    {
	return;
    }
    
    size_t block_want = TAR_BLOCK_SIZE - block_have;

    size_t new_size = range_count(*output) + block_want;

    buffer_resize(*output, new_size);

    memset(output->end - block_want, 0, block_want);
}

void tar_write_end (buffer_char * output)
{
    size_t add_size = 2 * TAR_BLOCK_SIZE;
    size_t new_size = range_count(*output) + add_size;

    buffer_resize(*output, new_size);

    memset(output->end - add_size, 0, add_size);
}

keyargs_define(tar_write_path_header)
{
    assert (args.path);
    assert (args.output);
    
    struct stat s;

    if (-1 == lstat (args.path, &s))
    {
	perror (args.path);
	log_fatal ("Failed to stat %s", args.path);
    }

    tar_type type =
	S_ISREG(s.st_mode) ? TAR_FILE
	: S_ISDIR(s.st_mode) ? TAR_DIR
	: S_ISLNK(s.st_mode) ? TAR_SYMLINK
	: TAR_ERROR;

    if (type == TAR_ERROR)
    {
	log_fatal ("Could not identify a tar entry type for %s", args.path);
    }

    char linkname[PATH_MAX + 1];

    if (type == TAR_SYMLINK)
    {
	int linkname_length = readlink(args.path, linkname, sizeof(linkname));
	
	if (linkname_length < 0)
	{
	    log_fatal ("linkname failed");
	}
	
	if (linkname_length == sizeof(linkname))
	{
	    log_fatal ("linkname exceeds buffer size");
	}
	
	linkname[linkname_length] = '\0';
    }
    
    if (!tar_write_header (.output = args.output,
			   .name = args.override_name ? args.override_name : args.path,
			   .mode = s.st_mode,
			   .uid = s.st_uid,
			   .gid = s.st_gid,
			   .size = s.st_size,
			   .type = type,
			   .linkname = (type == TAR_SYMLINK) ? linkname : NULL))
    {
	log_fatal ("Failed to write tar header");
    }

    if (args.detect_type)
    {
	*args.detect_type = type;
    }

    if (args.detect_size)
    {
	*args.detect_size = s.st_size;
    }

    return true;
    
fail:
    return false;
}
