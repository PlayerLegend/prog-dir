#ifndef FLAT_INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define FLAT_INCLUDES
#include "../array/range.h"
#include "../array/buffer.h"
#include "../keyargs/keyargs.h"
#endif

/** @file tar.h
    Describes the public interface for the tar library
*/

#define TAR_BLOCK_SIZE 512 ///< The size of a tar block

typedef enum tar_type tar_type;
enum tar_type { TAR_ERROR, TAR_DIR, TAR_FILE, TAR_SYMLINK, TAR_HARDLINK, TAR_END, TAR_LONGNAME, TAR_LONGLINK }; ///< Possible contents of a tar file

typedef struct  {
    bool ready; ///< True if the state is ready for use
    
    tar_type type; ///< The current item type
    buffer_char path; ///< The path of the current item
    buffer_char tmp; ///< A temporary buffer used by tar_update_fd

    size_t mode; ///< The mode of the current item

    struct {
	buffer_char path; ///< If the current item is a hardlink or symlink, this is its target path
    } link;

    struct {
	size_t size; ///< If the current item is a file, this is its size
    } file;
}
    tar_state; ///< Describes a single item in a tar file

// reading

void tar_restart(tar_state * state);
/**<
   @brief Restarts the given state so that it may be used to read a different tar file.
   @param state The state to be restarted
*/

bool tar_update_fd (tar_state * state, int fd);
/**<
   @brief Reads the first or next item from the file descriptor into the given state
   @return True if success, false otherwise
   @param state The state to be updated
   @param fd The file descriptor to read from
*/

bool tar_update_mem (tar_state * state, range_const_char * rest, const range_const_char * mem);
/**<
   @brief Adds one chunk from the beginning of mem to the given tar state.
   @return True if success, false otherwise
   @param state The state to be updated
   @param rest A range of bytes that will be pointed to the portion of mem following the chunk read by this function. Pass NULL here if you do not need that.
   @param mem A range of input bytes
*/

void tar_cleanup (tar_state * state);
/**<
   @brief Frees all memory allocated to the given state, but not the state itself.
   @param state The state to be cleaned
*/

keyargs_declare(bool,tar_read_region,
		buffer_char * buffer;
		size_t size;
		int fd;);
#define tar_read_region(...) keyargs_call(tar_read_region, __VA_ARGS__) 
/**<
   @brief Reads a stream of tar chunks, discarding any padding at the end. Use this for reading the contents of a tar file after the tar state indicates that there is one.
   @return True if success, false otherwise
   @param buffer The buffer in which to store the output
   @param size The size of the output to be read
   @param fd The file descriptor to read from
*/

bool tar_skip_file (tar_state * state, int fd);

// writing

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

void tar_write_padding (buffer_char * output, unsigned long long file_size);

void tar_write_end (buffer_char * output);

keyargs_declare(bool,tar_write_path_header,
		buffer_char * output;
		tar_type * detect_type;
		unsigned long long * detect_size;
		const char * path;
		const char * override_name;);

#define tar_write_path_header(...) keyargs_call(tar_write_path_header, __VA_ARGS__)

// compressed

typedef enum {
    TAR_COMPRESSION_NONE,
    TAR_COMPRESSION_DZIP,
}
    tar_compression_type;

