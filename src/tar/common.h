#ifndef FLAT_INCLUDES
#define FLAT_INCLUDES
#endif

/**
   @file tar/common.h
   This file creates definitions used by both the reading and writing portions of the tar library. Functions to read and write tar files can be found in tar/read.h and tar/write.h respectively.
*/

#define TAR_BLOCK_SIZE 512 ///< The size of a tar block

typedef enum
{
    TAR_ERROR, ///< Indicates an error state
    TAR_DIR, ///< Indicates a directory
    TAR_FILE, ///< Indicates a file
    TAR_SYMLINK, ///< Indicates a symlink
    TAR_HARDLINK, ///< Indicates a hardlink
    TAR_END, ///< Indicates the end of a tar file
    TAR_LONGNAME, ///< Indicates a longname that must be applied to the next tar item that isn't a longlink
    TAR_LONGLINK ///< Indicates a longlink that must be applied to the next hardlink or symlink
}
    tar_type; ///< Item types which may be found in a tar file
