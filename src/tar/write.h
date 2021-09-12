#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#include "common.h"
#endif

/**
   @file tar/write.h
   This file describes the public interface for the writing portion of the tar library.
   To write a tar file, first use tar_write_header or tar_write_path_header to generate a header for the item you are trying to write to your tar file. Then, if this item is a file, longname, or longlink, write its contents to the tar and use tar_write_padding to write the necessary padding after it. To terminate the tar file, use tar_write_end.
*/

keyargs_declare(bool,tar_write_header,
		buffer_char * output;
		const char * name;
		int mode;
		int uid;
		int gid;
		unsigned long long size;
		unsigned long long mtime;
		tar_type type;
		const char * linkname;
		const char * uname;
		const char * gname;
    );
#define tar_write_header(...) keyargs_call(tar_write_header, __VA_ARGS__)
/**<
   @brief This is a keyargs function used to write a tar header into an output buffer.
   @return True if successful, false otherwise
   @param output This is the output buffer into which the header will be written. 
   @param name This is the full path of the item described by the new header.
   @param mode This is the permissions mode to be indicated by the new header.
   @param uid This is the user id to be indicated by the new header. It can be left as 0 if uname is given as an argument.
   @param gid This is the group id to be indicated by the new header. It can be left as 0 if gname is given as an argument.
   @param size If the new header is describing a file, longname, or longlink item, then this will indicate the size of the item. For other types of tar items, this should be left as 0.
   @param mtime The last time that the file was modified, in epoch seconds.
   @param type The type of this item - this argument accepts values enumerated in tar_type. However, TAR_ERROR and TAR_END may not be used here, and their use will result in an error.
   @param linkname If the new header is describing a hardlink or symlink, this indicates the destination it points to.
   @param uname This is the user name to be indicated by the new header. It overrides uid if both are given.
   @param gname This is the group name to be indicated by the new header. It overrides gid if both are given.
*/

void tar_write_padding (buffer_char * output, unsigned long long file_size);
/**<
   Writes the necessary padding after a file of size 'file_size' into the given output buffer.
*/

void tar_write_end (buffer_char * output);
/**<
   Writes a terminating sequence of tar sectors into the given output buffer, these will indicate the end of a tar file.
*/

keyargs_declare(bool,tar_write_path_header,
		buffer_char * output;
		tar_type * detect_type;
		unsigned long long * detect_size;
		const char * path;
		const char * override_name;);
#define tar_write_path_header(...) keyargs_call(tar_write_path_header, __VA_ARGS__)
/**<
   Given a path to an existing file, directory, or symlink, this automatically generates a header based on the entity found at the given path.
   @param output The output buffer into which the new header will be written
   @param detect_type If a non-null pointer is given here, its destination will be assigned to the detected tar_type of the entity located at the given path.
   @param detect_size If a non-null pointer is given here, then its destination will be assigned to the size obtained from the stat function at the given path.
*/
